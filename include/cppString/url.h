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

#ifndef CPPSTRING_URL_H_
#define CPPSTRING_URL_H_

#include "cppString.h"
#include <vector>
#include <utility>

namespace sp
{
	// RFC 3986 URI parsing and construction. Components are stored exactly
	// as written (percent escapes are NOT decoded); use query_params() and
	// encode()/decode() when you need the decoded values.
	namespace url
	{
		class CPPSTRING_EXPORT Url
		{
		public:
			Url() = default;
			explicit Url(const cppString& text);

			[[nodiscard]] cppString scheme() const   { return m_scheme; }
			[[nodiscard]] cppString userinfo() const { return m_userinfo; }
			[[nodiscard]] cppString host() const     { return m_host; }
			[[nodiscard]] cppString port() const     { return m_port; }
			[[nodiscard]] cppString path() const     { return m_path; }
			[[nodiscard]] cppString query() const    { return m_query; }
			[[nodiscard]] cppString fragment() const { return m_fragment; }

			void set_scheme(const cppString& value)   { m_scheme = value; }
			void set_userinfo(const cppString& value) { m_userinfo = value; }
			void set_host(const cppString& value)     { m_host = value; }
			void set_port(const cppString& value)     { m_port = value; }
			void set_path(const cppString& value)     { m_path = value; }
			// Setting either component marks it as explicitly defined, so an
			// empty query/fragment still serializes as "?" / "#".
			void set_query(const cppString& value)    { m_query = value; m_has_query = true; }
			void set_fragment(const cppString& value) { m_fragment = value; m_has_fragment = true; }

			// RFC 3986 allows an authority component that is present but empty
			// ("file:///tmp/a", "http:///p"). Keep the component-based check
			// for URLs assembled with set_host()/set_port(), but also remember
			// whether "//" was actually present when parsing.
			[[nodiscard]] bool has_authority() const noexcept
			{
				return m_has_authority
					|| !m_host.empty() || !m_userinfo.empty() || !m_port.empty();
			}

			// Decoded query parameters, in order. Empty segments are skipped;
			// a segment without '=' yields an empty value.
			[[nodiscard]] std::vector<std::pair<cppString, cppString>> query_params() const;
			void set_query_params(const std::vector<std::pair<cppString, cppString>>& params);

			[[nodiscard]] cppString query_param(const cppString& key,
				const cppString& fallback = cppString{}) const;
			void set_query_param(const cppString& key, const cppString& value);
			[[nodiscard]] bool has_query_param(const cppString& key) const;

			// Rebuild the URI string from its components.
			[[nodiscard]] cppString str() const;

			// Resolve a (possibly relative) reference against this URL,
			// following RFC 3986 section 5.2 including dot-segment removal.
			[[nodiscard]] Url resolve(const cppString& reference) const;

		private:
			cppString m_scheme{};
			cppString m_userinfo{};
			cppString m_host{};
			cppString m_port{};
			cppString m_path{};
			cppString m_query{};
			cppString m_fragment{};
			// RFC 3986 distinguishes an undefined component from an empty one:
			// "?" defines an empty query, "#" an empty fragment, and "//" an
			// authority whose host/userinfo/port may all be empty.
			bool m_has_query = false;
			bool m_has_fragment = false;
			bool m_has_authority = false;
		};

		[[nodiscard]] Url parse(const cppString& text);

		// Thin wrappers over coding::urlEncode / urlDecode. decode() throws
		// Invalid_Conversion on a malformed percent escape.
		[[nodiscard]] cppString encode(const cppString& text);
		[[nodiscard]] cppString decode(const cppString& text);
	}
}

#endif
