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

#ifndef CPPSTRING_LANGCVT_JAVACVT_H_
#define CPPSTRING_LANGCVT_JAVACVT_H_

#include <cppString/langcvt/langcvt.h>

namespace cppstr::javacvt
{
	struct CPPSTRING_EXPORT Options : langcvt::CommonOptions
	{
		cppString runtime{};        // empty = auto-detect java
		cppString class_name{};     // empty = detect from the public class declaration
	};

	// First available Java launcher name. Never executes anything.
	[[nodiscard]] cppString default_runtime();

	// Write `source` to a unique temporary .java file and run it with the
	// Java 11+ single-file source launcher (`java File.java`). The temporary
	// directory is removed before returning. A missing runtime is reported
	// through Result; an unusable class declaration throws Invalid_Argument.
	[[nodiscard]] langcvt::Result run(const cppString& source,
		const Options& options = {});

	// run() returning stdout only. Throws Invalid_Operation on failure.
	[[nodiscard]] cppString eval(const cppString& source,
		const Options& options = {});
}

#endif // CPPSTRING_LANGCVT_JAVACVT_H_
