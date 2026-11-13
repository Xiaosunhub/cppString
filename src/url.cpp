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

#include "url.h"
#include "coding.h"
#include <cctype>

namespace
{
	bool is_alpha(char c) noexcept
	{
		return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
	}

	bool all_digits(const String& s) noexcept
	{
		if (s.empty())
			return false;

		for (const char c : s)
		{
			if (c < '0' || c > '9')
				return false;
		}

		return true;
	}

	void split_host_port(const String& authority, sp::cppString& host, sp::cppString& port)
	{
		if (!authority.empty() && authority.front() == '[')
		{
			const std::size_t close = authority.find(']');

			if (close == String::npos)
			{
				host = sp::cppString{ authority };
				return;
			}

			host = sp::cppString{ authority.substr(0, close + 1) };

			if (close + 1 < authority.size() && authority[close + 1] == ':')
				port = sp::cppString{ authority.substr(close + 2) };

			return;
		}

		const std::size_t colon = authority.rfind(':');

		if (colon != String::npos && all_digits(authority.substr(colon + 1)))
		{
			host = sp::cppString{ authority.substr(0, colon) };
			port = sp::cppString{ authority.substr(colon + 1) };
		}
		else
		{
			host = sp::cppString{ authority };
		}
	}

	// RFC 3986 section 5.2.4, implemented segment-wise.
	String remove_dot_segments(const String& path)
	{
		const bool absolute = !path.empty() && path.front() == '/';
		// RFC 3986 5.2.4 replaces a trailing "/." or "/.." with "/", so the
		// result ends in a slash even though the input does not.
		const bool trailing = (path.size() > 1 && path.back() == '/')
			|| path.ends_with("/.") || path.ends_with("/..");

		std::vector<String> stack;
		std::size_t pos = 0;

		while (pos < path.size())
		{
			const std::size_t end = path.find('/', pos);
			String part = path.substr(pos, end == String::npos ? String::npos : end - pos);

			if (part == ".")
			{
				// drop
			}
			else if (part == "..")
			{
				if (!stack.empty() && stack.back() != "..")
					stack.pop_back();
				else if (!absolute)
					stack.push_back(part);
			}
			else if (!part.empty())
			{
				stack.push_back(std::move(part));
			}

			if (end == String::npos)
				break;

			pos = end + 1;
		}

		String result;

		if (absolute)
			result += '/';

		for (std::size_t i = 0; i < stack.size(); ++i)
		{
			if (i > 0)
				result += '/';

			result += stack[i];
		}

		if (trailing && !result.empty() && result.back() != '/')
			result += '/';

		return result;
	}

	String merge_paths(const String& base, const String& reference, bool base_has_authority)
	{
		// RFC 3986 5.2.3: with an authority and an empty base path the merged
		// path must start with "/", otherwise host and path run together.
		if (base_has_authority && base.empty())
			return String{ "/" } + reference;

		if (!base.empty() && base.find('/') == String::npos)
			return reference;

		const std::size_t slash = base.rfind('/');
		String merged = (slash == String::npos) ? String{} : base.substr(0, slash + 1);
		merged += reference;
		return merged;
	}
}

sp::url::Url::Url(const cppString& text)
{
	String rest = text.s_str();

	if (const std::size_t hash = rest.find('#'); hash != String::npos)
	{
		m_fragment = cppString{ rest.substr(hash + 1) };
		m_has_fragment = true;
		rest.resize(hash);
	}

	if (const std::size_t question = rest.find('?'); question != String::npos)
	{
		m_query = cppString{ rest.substr(question + 1) };
		m_has_query = true;
		rest.resize(question);
	}

	if (!rest.empty() && is_alpha(rest.front()))
	{
		const std::size_t colon = rest.find(':');
		const std::size_t stop = rest.find_first_of("/?#");

		if (colon != String::npos && (stop == String::npos || colon < stop))
		{
			m_scheme = cppString{ rest.substr(0, colon) };
			rest = rest.substr(colon + 1);
		}
	}

	if (rest.size() >= 2 && rest[0] == '/' && rest[1] == '/')
	{
		// The authority is defined even when every one of its subcomponents
		// is empty (e.g. "file:///tmp/a"). str() must keep the leading "//".
		m_has_authority = true;
		const std::size_t end = rest.find('/', 2);
		String authority = rest.substr(2, end == String::npos ? String::npos : end - 2);
		rest = (end == String::npos) ? String{} : rest.substr(end);

		if (const std::size_t at = authority.rfind('@'); at != String::npos)
		{
			m_userinfo = cppString{ authority.substr(0, at) };
			authority = authority.substr(at + 1);
		}

		split_host_port(authority, m_host, m_port);
	}

	m_path = cppString{ rest };
}

std::vector<std::pair<sp::cppString, sp::cppString>> sp::url::Url::query_params() const
{
	std::vector<std::pair<cppString, cppString>> result;
	const String& src = m_query.s_str();
	std::size_t pos = 0;

	while (pos <= src.size())
	{
		const std::size_t amp = src.find('&', pos);
		String part = src.substr(pos, amp == String::npos ? String::npos : amp - pos);

		if (!part.empty())
		{
			const std::size_t eq = part.find('=');
			String key = (eq == String::npos) ? part : part.substr(0, eq);
			String value = (eq == String::npos) ? String{} : part.substr(eq + 1);
			result.emplace_back(decode(cppString{ key }), decode(cppString{ value }));
		}

		if (amp == String::npos)
			break;

		pos = amp + 1;
	}

	return result;
}

void sp::url::Url::set_query_params(
	const std::vector<std::pair<cppString, cppString>>& params)
{
	String out;

	for (std::size_t i = 0; i < params.size(); ++i)
	{
		if (i > 0)
			out += '&';

		out += encode(params[i].first).s_str();
		out += '=';
		out += encode(params[i].second).s_str();
	}

	m_query = cppString(std::move(out));
	m_has_query = true;
}

sp::cppString sp::url::Url::query_param(const cppString& key, const cppString& fallback) const
{
	for (const auto& param : query_params())
	{
		if (param.first == key)
			return param.second;
	}

	return fallback;
}

bool sp::url::Url::has_query_param(const cppString& key) const
{
	for (const auto& param : query_params())
	{
		if (param.first == key)
			return true;
	}

	return false;
}

void sp::url::Url::set_query_param(const cppString& key, const cppString& value)
{
	const auto params = query_params();
	std::vector<std::pair<cppString, cppString>> updated;
	updated.reserve(params.size());
	bool replaced = false;

	for (const auto& param : params)
	{
		if (param.first == key)
		{
			// A query may contain the same key more than once. Update the
			// first occurrence and drop the rest so one set() does not leave
			// several identical "key=value" pairs behind.
			if (replaced)
				continue;

			updated.emplace_back(param.first, value);
			replaced = true;
		}
		else
		{
			updated.emplace_back(param.first, param.second);
		}
	}

	if (!replaced)
		updated.emplace_back(key, value);

	set_query_params(updated);
}

sp::cppString sp::url::Url::str() const
{
	String out;

	if (!m_scheme.empty())
	{
		out += m_scheme.s_str();
		out += ':';
	}

	if (has_authority())
	{
		out += "//";

		if (!m_userinfo.empty())
		{
			out += m_userinfo.s_str();
			out += '@';
		}

		out += m_host.s_str();

		if (!m_port.empty())
		{
			out += ':';
			out += m_port.s_str();
		}
	}

	out += m_path.s_str();

	if (m_has_query)
	{
		out += '?';
		out += m_query.s_str();
	}

	if (m_has_fragment)
	{
		out += '#';
		out += m_fragment.s_str();
	}

	return cppString(std::move(out));
}

sp::url::Url sp::url::Url::resolve(const cppString& reference) const
{
	Url ref{ reference };
	Url result;

	if (!ref.m_scheme.empty())
	{
		result = ref;
		result.m_path = cppString{ remove_dot_segments(ref.m_path.s_str()) };
		return result;
	}

	result.m_scheme = m_scheme;
	result.m_fragment = ref.m_fragment;
	result.m_has_fragment = ref.m_has_fragment;

	if (ref.has_authority())
	{
		// A reference such as "///p" defines an empty authority: clear the
		// base host/port instead of inheriting them.
		result.m_has_authority = true;
		result.m_userinfo = ref.m_userinfo;
		result.m_host = ref.m_host;
		result.m_port = ref.m_port;
		result.m_path = cppString{ remove_dot_segments(ref.m_path.s_str()) };
		result.m_query = ref.m_query;
		result.m_has_query = ref.m_has_query;
		return result;
	}

	result.m_userinfo = m_userinfo;
	result.m_host = m_host;
	result.m_port = m_port;
	// A relative reference keeps the base authority, including the case
	// where that authority is present but empty.
	result.m_has_authority = has_authority();

	if (ref.m_path.empty())
	{
		result.m_path = cppString{ remove_dot_segments(m_path.s_str()) };

		// RFC 3986 5.2.2: only an *undefined* query inherits the base query.
		// "?" defines an empty query and must clear it.
		if (ref.m_has_query)
		{
			result.m_query = ref.m_query;
			result.m_has_query = true;
		}
		else
		{
			result.m_query = m_query;
			result.m_has_query = m_has_query;
		}
	}
	else
	{
		String merged = ref.m_path.s_str().front() == '/'
			? ref.m_path.s_str()
			: merge_paths(m_path.s_str(), ref.m_path.s_str(), has_authority());

		result.m_path = cppString{ remove_dot_segments(merged) };
		result.m_query = ref.m_query;
		result.m_has_query = ref.m_has_query;
	}

	return result;
}

sp::url::Url sp::url::parse(const cppString& text)
{
	return Url{ text };
}

sp::cppString sp::url::encode(const cppString& text)
{
	return coding::urlEncode(text);
}

sp::cppString sp::url::decode(const cppString& text)
{
	return coding::urlDecode(text);
}
