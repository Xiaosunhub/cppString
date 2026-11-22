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

#include <cppString/langcvt/langcvt.h>
#include <cppString/exec.h>

namespace cppstr::langcvt
{
	bool interpreter_exists(const cppString& program)
	{
		return exec::exists(program);
	}

	Result run_argv(const std::vector<cppString>& argv, const CommonOptions& options)
	{
		exec::Options eo;
		eo.working_directory = options.working_directory;
		eo.timeout = options.timeout;
		eo.merge_stderr = options.merge_stderr;
		return exec::invoke(argv, eo);
	}

	cppString eval_result(const Result& result, const char* action)
	{
		if (result.ok())
			return result.out;

		cppString message = cppString("langcvt: ") + action + " failed";
		if (!result.out.empty())
			message += "\nstdout: " + result.out;
		if (!result.err.empty())
			message += "\nstderr: " + result.err;
		if (result.timed_out)
			message += "\n(the process timed out)";
		else if (!result.started)
			message += "\n(the interpreter could not be started)";
		throw Invalid_Operation(message.s_str());
	}
}
