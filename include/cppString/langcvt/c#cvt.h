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

#ifndef CPPSTRING_LANGCVT_CSHARPCVT_H_
#define CPPSTRING_LANGCVT_CSHARPCVT_H_

#include <cppString/langcvt/langcvt.h>

namespace cppstr::csharpcvt
{
	struct CPPSTRING_EXPORT Options : langcvt::CommonOptions
	{
		cppString compiler{};               // empty = auto-detect csc (incl. the .NET
		                                    // Framework install path on Windows) / mcs
		std::vector<cppString> arguments{}; // extra compile arguments
		// Program used to launch the compiled assembly (prepended before the
		// executable path). Empty on Windows, where csc produces a native
		// executable; set "mono" or "dotnet" on POSIX.
		cppString runner{};
	};

	// First available C# compiler name. Never executes anything.
	[[nodiscard]] cppString default_compiler();

	// Write `source` to a unique temporary .cs file, compile it and run the
	// produced assembly (through Options::runner when set). A failing
	// compilation is reported through the returned Result (ok() is false,
	// diagnostics in out / err); the assembly is never started. The temporary
	// directory is removed before returning.
	[[nodiscard]] langcvt::Result run(const cppString& source,
		const Options& options = {});

	// run() returning stdout only. Throws Invalid_Operation on failure.
	[[nodiscard]] cppString eval(const cppString& source,
		const Options& options = {});
}

#endif // CPPSTRING_LANGCVT_CSHARPCVT_H_
