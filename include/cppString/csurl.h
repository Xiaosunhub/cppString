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

#ifndef CPPSTRING_CSURL_H_
#define CPPSTRING_CSURL_H_

#include "cppString.h"

namespace sp
{
	// Minimal synchronous HTTP/HTTPS client for reading web pages and APIs.
	//
	// No third-party dependency and no curl: Windows builds use the system
	// WinHTTP API (HTTP + HTTPS); other platforms use a small POSIX socket
	// client over plain HTTP. TLS on POSIX is intentionally out of scope and
	// an https:// URL there throws Invalid_Argument with an explicit message.
	//
	// URL parsing reuses sp::url. Only the http and https schemes are accepted.
	namespace csurl
	{
		struct Options
		{
			int timeout_ms = 15000;             // per-operation timeout (ms); > 0
			cppString user_agent = "cppString-csurl/1.0";
			std::size_t max_bytes = 8u * 1024u * 1024u; // body cap; 0 = unlimited
			bool follow_redirects = true;
			int max_redirects = 5;
		};

		struct Response
		{
			int status = 0;                     // HTTP status; 0 = transport failure
			cppString body;                     // response body (raw bytes)
			cppString content_type;             // Content-Type header, if any
			cppString final_url;                // URL after redirects
			cppString error;                    // transport error text when status == 0

			[[nodiscard]] bool ok() const noexcept
			{
				return status >= 200 && status < 300;
			}
		};

		// Perform a GET. Transport failures do NOT throw: they return a Response
		// with status == 0 and a non-empty error(). Malformed URLs, unsupported
		// schemes and invalid options throw Invalid_Argument.
		[[nodiscard]] Response get(const cppString& url, const Options& options = Options{});

		// Convenience wrapper: returns body on a 2xx response, otherwise throws
		// Invalid_Operation with status and error context.
		[[nodiscard]] cppString fetch(const cppString& url, const Options& options = Options{});

		// True when this build can perform HTTPS requests (Windows/WinHTTP).
		[[nodiscard]] bool supports_https() noexcept;
	}
}

#endif // CPPSTRING_CSURL_H_
