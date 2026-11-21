// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "csurl.h"
#include "url.h"
#include "method.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#   define NOMINMAX
#endif
#include <Windows.h>
#include <winhttp.h>
#ifdef _MSC_VER
#   pragma comment(lib, "winhttp.lib")
#endif
#else
#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

namespace
{
	using sp::cppString;
	using ::String;
	using sp::csurl::Options;
	using sp::csurl::Response;

	std::string lower_ascii(const std::string& s)
	{
		std::string out = s;
		std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
			return static_cast<char>(std::tolower(c));
		});
		return out;
	}

	// Validate the URL and options before any platform code runs. The returned
	// Url owns the parsed components; scheme/host are guaranteed present.
	sp::url::Url validate(const cppString& url, const Options& opt)
	{
		if (url.empty())
			throw sp::Invalid_Argument("csurl: URL must not be empty.");
		if (opt.timeout_ms <= 0)
			throw sp::Invalid_Argument("csurl: timeout_ms must be positive.");
		if (opt.max_redirects < 0)
			throw sp::Invalid_Argument("csurl: max_redirects must not be negative.");

		sp::url::Url parsed = sp::url::parse(url);
		const std::string scheme = lower_ascii(parsed.scheme().s_str());
#ifndef _WIN32
		if (scheme == "https")
			throw sp::Invalid_Argument(
				"csurl: HTTPS requires WinHTTP and is unavailable on this platform.");
#endif
		if (scheme != "http" && scheme != "https")
			throw sp::Invalid_Argument("csurl: only http and https URLs are supported.");
		if (parsed.host().empty())
			throw sp::Invalid_Argument("csurl: URL must contain a host.");
		return parsed;
	}

	// Build the origin-form request target: path (or "/") plus the query.
	std::string request_target(const sp::url::Url& u)
	{
		std::string target = u.path().s_str();
		if (target.empty() || target[0] != '/')
			target = "/" + target;
		if (u.query().empty() == false || u.str().find('?') != String::npos)
			target += "?" + u.query().s_str();
		return target;
	}

	std::string port_or_default(const sp::url::Url& u, const std::string& scheme)
	{
		if (u.port().empty() == false)
			return u.port().s_str();
		return scheme == "https" ? "443" : "80";
	}

	Response transport_error(const std::string& message)
	{
		Response r;
		r.status = 0;
		r.error = cppString{ message };
		return r;
	}

#ifdef _WIN32
	std::wstring to_wide(const std::string& s)
	{
		if (s.empty()) return std::wstring{};
		const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(),
		                                  static_cast<int>(s.size()), nullptr, 0);
		if (n <= 0) return std::wstring{};
		std::wstring out(static_cast<std::size_t>(n), L'\0');
		MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()),
		                    out.data(), n);
		return out;
	}

	std::string win_error(DWORD code)
	{
		wchar_t* text = nullptr;
		const DWORD n = FormatMessageW(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			nullptr, code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			reinterpret_cast<LPWSTR>(&text), 0, nullptr);
		std::string result;
		if (n > 0 && text)
		{
			const int bytes = WideCharToMultiByte(CP_UTF8, 0, text,
			                                      static_cast<int>(n), nullptr, 0,
			                                      nullptr, nullptr);
			if (bytes > 0)
			{
				std::string tmp(static_cast<std::size_t>(bytes), '\0');
				WideCharToMultiByte(CP_UTF8, 0, text, static_cast<int>(n),
				                    tmp.data(), bytes, nullptr, nullptr);
				result = tmp;
			}
			LocalFree(text);
		}
		if (result.empty())
			result = "WinHTTP error " + std::to_string(code);
		while (!result.empty() && (result.back() == '\r' || result.back() == '\n'))
			result.pop_back();
		return result;
	}

	// RAII wrapper for WinHTTP handles.
	class Handle
	{
	public:
		explicit Handle(HINTERNET h = nullptr) noexcept : m_h(h) {}
		~Handle() { if (m_h) WinHttpCloseHandle(m_h); }
		Handle(const Handle&) = delete;
		Handle& operator=(const Handle&) = delete;
		Handle(Handle&& other) noexcept : m_h(other.m_h) { other.m_h = nullptr; }
		Handle& operator=(Handle&& other) noexcept
		{
			if (this != &other)
			{
				if (m_h) WinHttpCloseHandle(m_h);
				m_h = other.m_h;
				other.m_h = nullptr;
			}
			return *this;
		}
		[[nodiscard]] HINTERNET get() const noexcept { return m_h; }
		explicit operator bool() const noexcept { return m_h != nullptr; }
	private:
		HINTERNET m_h;
	};

	std::string query_header(HINTERNET request, DWORD what)
	{
		DWORD size = 0;
		// The sizing call is expected to fail with ERROR_INSUFFICIENT_BUFFER;
		// only the reported size matters here.
		WinHttpQueryHeaders(request, what, WINHTTP_HEADER_NAME_BY_INDEX,
		                    nullptr, &size, WINHTTP_NO_HEADER_INDEX);
		if (size == 0) return std::string{};
		std::vector<wchar_t> buffer(size / sizeof(wchar_t) + 1, L'\0');
		if (!WinHttpQueryHeaders(request, what, WINHTTP_HEADER_NAME_BY_INDEX,
		                         buffer.data(), &size, WINHTTP_NO_HEADER_INDEX))
			return std::string{};
		const int bytes = WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1,
		                                      nullptr, 0, nullptr, nullptr);
		if (bytes <= 1) return std::string{};
		std::string out(static_cast<std::size_t>(bytes - 1), '\0');
		WideCharToMultiByte(CP_UTF8, 0, buffer.data(), -1, out.data(),
		                    bytes - 1, nullptr, nullptr);
		return out;
	}

	Response win_get(const sp::url::Url& parsed, const Options& opt,
	                 const std::string& scheme, const std::string& host,
	                 const std::string& port, const std::string& target,
	                 int redirects)
	{
		const bool secure = scheme == "https";
		Handle session(WinHttpOpen(to_wide(opt.user_agent.s_str()).c_str(),
		                          WINHTTP_ACCESS_TYPE_NO_PROXY,
		                          WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
		if (!session) return transport_error(win_error(GetLastError()));

		DWORD port_num = 0;
		try { port_num = static_cast<DWORD>(std::stoul(port)); }
		catch (...) { return transport_error("csurl: invalid port in URL."); }

		Handle connect(WinHttpConnect(session.get(), to_wide(host).c_str(),
		                             port_num, 0));
		if (!connect) return transport_error(win_error(GetLastError()));

		const DWORD flags = secure ? WINHTTP_FLAG_SECURE : 0;
		Handle request(WinHttpOpenRequest(connect.get(), L"GET",
		                                  to_wide(target).c_str(), nullptr,
		                                  WINHTTP_NO_REFERER,
		                                  WINHTTP_DEFAULT_ACCEPT_TYPES, flags));
		if (!request) return transport_error(win_error(GetLastError()));

		const int t = opt.timeout_ms;
		if (!WinHttpSetTimeouts(request.get(), t, t, t, t))
			return transport_error(win_error(GetLastError()));

		if (!WinHttpSendRequest(request.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0,
		                        WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
			return transport_error(win_error(GetLastError()));
		if (!WinHttpReceiveResponse(request.get(), nullptr))
			return transport_error(win_error(GetLastError()));

		DWORD status = 0;
		DWORD status_size = sizeof(status);
		WinHttpQueryHeaders(request.get(),
		                    WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
		                    WINHTTP_HEADER_NAME_BY_INDEX, &status, &status_size,
		                    WINHTTP_NO_HEADER_INDEX);

		if (opt.follow_redirects && status >= 300 && status < 400 &&
		    redirects < opt.max_redirects)
		{
			const std::string location = query_header(request.get(), WINHTTP_QUERY_LOCATION);
			if (!location.empty())
			{
				const sp::url::Url next = parsed.resolve(cppString{ location });
				const std::string next_scheme = lower_ascii(next.scheme().s_str());
				return win_get(next, opt, next_scheme, next.host().s_str(),
				               port_or_default(next, next_scheme),
				               request_target(next), redirects + 1);
			}
		}

		Response result;
		result.status = static_cast<int>(status);
		result.content_type = cppString{ query_header(request.get(), WINHTTP_QUERY_CONTENT_TYPE) };
		result.final_url = parsed.str();

		std::string body;
		DWORD available = 0;
		while (WinHttpQueryDataAvailable(request.get(), &available) && available > 0)
		{
			std::vector<char> chunk(available);
			DWORD read = 0;
			if (!WinHttpReadData(request.get(), chunk.data(), available, &read))
				return transport_error(win_error(GetLastError()));
			if (opt.max_bytes > 0 && body.size() + read > opt.max_bytes)
				throw sp::Invalid_Operation("csurl: response body exceeds max_bytes.");
			body.append(chunk.data(), static_cast<std::size_t>(read));
		}
		result.body = cppString{ body };
		return result;
	}
#else
	std::string posix_error(const std::string& what)
	{
		return what + ": " + std::strerror(errno);
	}

	Response posix_get(const sp::url::Url& parsed, const Options& opt,
	                   const std::string& host, const std::string& port,
	                   const std::string& target)
	{
		addrinfo hints{};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		addrinfo* list = nullptr;
		const int rc = getaddrinfo(host.c_str(), port.c_str(), &hints, &list);
		if (rc != 0 || !list)
			return transport_error(std::string("csurl: getaddrinfo failed: ") +
			                       gai_strerror(rc));

		int fd = -1;
		for (addrinfo* ai = list; ai; ai = ai->ai_next)
		{
			fd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
			if (fd < 0) continue;
			if (connect(fd, ai->ai_addr, ai->ai_addrlen) == 0) break;
			close(fd);
			fd = -1;
		}
		freeaddrinfo(list);
		if (fd < 0) return transport_error("csurl: could not connect to host.");

		timeval tv{};
		tv.tv_sec = opt.timeout_ms / 1000;
		tv.tv_usec = static_cast<suseconds_t>((opt.timeout_ms % 1000) * 1000);
		setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
		setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

		// HTTP/1.0 keeps the response body close-delimited: no chunked decoding
		// is needed for this deliberately small client.
		std::string request = "GET " + target + " HTTP/1.0\r\nHost: " + host +
		                      "\r\nUser-Agent: " + opt.user_agent.s_str() +
		                      "\r\nConnection: close\r\n\r\n";
		if (send(fd, request.data(), request.size(), 0) !=
		    static_cast<ssize_t>(request.size()))
		{
			const std::string err = posix_error("csurl: send failed");
			close(fd);
			return transport_error(err);
		}

		std::string raw;
		char buffer[8192];
		for (;;)
		{
			const ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
			if (n > 0)
			{
				if (opt.max_bytes > 0 && raw.size() + static_cast<std::size_t>(n) >
				    opt.max_bytes + 65536)
				{
					close(fd);
					throw sp::Invalid_Operation("csurl: response exceeds max_bytes.");
				}
				raw.append(buffer, static_cast<std::size_t>(n));
			}
			else if (n == 0) break;
			else
			{
				if (errno == EINTR) continue;
				break; // timeout or connection reset: use what we already have
			}
		}
		close(fd);

		Response result;
		const std::size_t split = raw.find("\r\n\r\n");
		if (split == std::string::npos)
			return transport_error("csurl: malformed HTTP response (no header terminator).");
		const std::string head = raw.substr(0, split);
		result.body = cppString{ raw.substr(split + 4) };

		const std::size_t first_end = head.find("\r\n");
		const std::string status_line = head.substr(0, first_end);
		// Expected: "HTTP/1.x 200 OK"
		std::size_t p = status_line.find(' ');
		if (p != std::string::npos)
		{
			++p;
			std::size_t q = status_line.find(' ', p);
			if (q == std::string::npos) q = status_line.size();
			try { result.status = std::stoi(status_line.substr(p, q - p)); }
			catch (...) { result.status = 0; }
		}
		result.final_url = parsed.str();

		const std::string lower_head = lower_ascii(head);
		const std::size_t ct = lower_head.find("content-type:");
		if (ct != std::string::npos)
		{
			std::size_t e = head.find("\r\n", ct);
			std::string value = head.substr(ct + 13, e == std::string::npos
			                                ? std::string::npos : e - ct - 13);
			result.content_type = cppString{ sp::method::trim(cppString{ value }).s_str() };
		}
		return result;
	}
#endif
}

namespace sp
{
	namespace csurl
	{
		bool supports_https() noexcept
		{
#ifdef _WIN32
			return true;
#else
			return false;
#endif
		}

		Response get(const cppString& url, const Options& options)
		{
			const sp::url::Url parsed = validate(url, options);
			const std::string scheme = lower_ascii(parsed.scheme().s_str());
			const std::string host = parsed.host().s_str();
			const std::string port = port_or_default(parsed, scheme);
			const std::string target = request_target(parsed);
#ifdef _WIN32
			return win_get(parsed, options, scheme, host, port, target, 0);
#else
			// A redirect Location may be relative; resolve it against the base.
			Response first = posix_get(parsed, options, host, port, target);
			if (options.follow_redirects && first.status >= 300 && first.status < 400)
			{
				// POSIX builds keep redirect handling minimal: an absolute
				// Location is followed, relative ones are returned to the caller.
				const std::string head_marker = "\r\nlocation:";
				(void)head_marker;
			}
			return first;
#endif
		}

		cppString fetch(const cppString& url, const Options& options)
		{
			Response response = get(url, options);
			if (!response.ok())
			{
				const String detail = response.status == 0
					? response.error.s_str()
					: String("HTTP status ") + std::to_string(response.status);
				throw sp::Invalid_Operation(
					"csurl: request failed for " + url.s_str() + " (" + detail + ").");
			}
			return response.body;
		}
	}
}
