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

#ifndef CPPSTRING_LANGCVT_H_
#define CPPSTRING_LANGCVT_H_

#include <cppString/cppString.h>
#include <cppString/exec.h>
#include <chrono>
#include <filesystem>
#include <functional>
#include <vector>

namespace cppstr
{
	// Execute source code written in another language and capture its result.
	// Source text is always passed as an argv element through exec::invoke(),
	// never through a shell, so quotes and newlines are preserved exactly.
	namespace langcvt
	{
		// Reuse the well-tested execution result: exit_code / out / err /
		// started / timed_out, plus ok() and output().
		using Result = cppstr::exec::Result;

		struct CPPSTRING_EXPORT CommonOptions
		{
			cppString working_directory{};                  // empty = inherit
			std::chrono::milliseconds timeout{ 0 };         // 0 = wait forever
			bool merge_stderr{ false };                     // redirect stderr to out
		};

		[[nodiscard]] bool interpreter_exists(const cppString& program);
		[[nodiscard]] Result run_argv(const std::vector<cppString>& argv,
			const CommonOptions& options = {});
		[[nodiscard]] cppString eval_result(const Result& result, const char* action);

		// Unique temporary workspace for the compiled-language adapters
		// (ccvt / cppcvt / rustcvt / csharpcvt). The directory is created in
		// the constructor and removed when the workspace is destroyed.
		struct CPPSTRING_EXPORT temp_workspace
		{
			std::filesystem::path directory;

			temp_workspace();
			~temp_workspace();
			temp_workspace(const temp_workspace&) = delete;
			temp_workspace& operator=(const temp_workspace&) = delete;
		};

		// True when the driver speaks MSVC syntax (cl, clang-cl), which takes
		// "/Fe<exe>" instead of "-o <exe>".
		[[nodiscard]] bool is_msvc_driver(const cppString& driver);

		// The path in its native (preferred-separator) form. Windows-native
		// tools such as csc treat "/"-prefixed components as options, so the
		// compiled-language adapters pass native paths to the compiler and to
		// the produced binary.
		[[nodiscard]] cppString native_path(const std::filesystem::path& path);

		// Shared compile-and-run pipeline for compiled languages. Creates a
		// temporary workspace, writes `source` into it as `source_name`, asks
		// `plan` for the compiler argv (given the source and executable paths),
		// compiles, and on success executes the produced binary with
		// `run_arguments` (optionally through `runner`, e.g. "mono", prepended
		// before the executable path).
		//
		// If compilation fails or times out, the returned Result describes the
		// compiler invocation: ok() is false and the diagnostics are in
		// out / err; the executable is never started. `options` applies to
		// each step independently (working directory, timeout, stderr merge).
		// Throws Invalid_Argument for empty source / empty compiler argv and
		// Invalid_Operation when the workspace cannot be created.
		[[nodiscard]] Result compile_and_run(
			const cppString& source,
			const cppString& source_name,
			const std::function<std::vector<cppString>(
				const std::filesystem::path& source_path,
				const std::filesystem::path& executable_path)>& plan,
			const std::vector<cppString>& run_arguments,
			const CommonOptions& options,
			const char* module_name,
			const cppString& runner = {});
	}
}

#endif // CPPSTRING_LANGCVT_H_
