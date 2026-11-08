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

#ifndef CPPSTRING_PATH_H_
#define CPPSTRING_PATH_H_

#include "cppString.h"

namespace sp
{
	// Pure string-level path handling: no filesystem access, no throw on missing files.
	// Accepts both '/' and '\\' as separators; every result uses '/'.
	namespace path
	{
		// Join base and child. An absolute child replaces base entirely.
		cppString join(const cppString& base, const cppString& child);

		// Collapse duplicate separators and resolve "." / ".." segments.
		// Keeps a drive prefix ("C:"), a leading "/" or a UNC "//server" prefix.
		// Returns "." for an empty path.
		cppString normalize(const cppString& p);

		// Last component: "a/b/c.txt" -> "c.txt"; a trailing separator is ignored.
		cppString filename(const cppString& p);

		// Filename without its last extension: "a/b/c.tar.gz" -> "c.tar";
		// a leading dot is not an extension (".gitignore" -> ".gitignore").
		cppString stem(const cppString& p);

		// Last extension including the dot, or "" when there is none.
		cppString extension(const cppString& p);

		// Directory part: "a/b/c.txt" -> "a/b"; "/a" -> "/"; "a" -> ".";
		cppString parent(const cppString& p);

		// True for "/x", "\\x", "C:/x", "C:x" and "//server/share".
		bool is_absolute(const cppString& p) noexcept;

		// Path of `to` relative to the directory `from`. Both sides are
		// normalized first. Returns `to` unchanged when the roots differ
		// (different drives, absolute vs relative). Equal paths give ".".
		cppString relative(const cppString& from, const cppString& to);
	}
}

#endif
