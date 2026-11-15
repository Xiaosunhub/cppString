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

#ifndef CPPSTRING_TOML_H_
#define CPPSTRING_TOML_H_

#include "cppString.h"
#include "json.h"

namespace sp
{
	// TOML 1.0 core subset reader/writer.
	//
	// The document model is sp::json::Value, so TOML shares one tree with the
	// JSON module. A document always parses to an Object. Supported constructs:
	// comments, bare / quoted / dotted keys, [table] headers, basic / literal /
	// multiline strings (with \\u and \\U escapes), decimal / hex / octal /
	// binary integers (including underscores), floats, inf / nan, booleans,
	// arrays (mixed types, trailing comma, comments) and inline tables.
	//
	// Deliberate subset limits, documented rather than hidden:
	//   * Date-time literals are preserved verbatim as String values; dump()
	//     emits them as ordinary quoted strings.
	//   * Array-of-tables ([[name]]) is rejected with Invalid_Format.
	//   * Numbers are stored as double (json::Value). Integers whose magnitude
	//     exceeds 2^53-1 are rejected instead of silently losing precision, and
	//     dump() emits integral numbers without a decimal point.
	//   * TOML has no null; dump() rejects Null values.
	//
	// Malformed input throws sp::Invalid_Format with line and byte position;
	// out-of-range numbers throw sp::Invalid_Conversion.
	namespace toml
	{
		// Parse a TOML document. Trailing content other than a comment is an
		// error. The returned root is always an Object.
		[[nodiscard]] json::Value parse(const cppString& text);

		// Deterministic serialization. Root-level scalars and simple tables
		// keep their position (simple tables are inlined); a root table that
		// contains another table gets a [section] written after them, and its
		// child tables use the conventional [a.b] form. Key insertion order
		// survives a dump/parse round trip.
		// Requires the root (and every table) to be an Object.
		[[nodiscard]] cppString dump(const json::Value& root);
	}
}

#endif
