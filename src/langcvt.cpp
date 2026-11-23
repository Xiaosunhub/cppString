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
#include <atomic>
#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

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

	temp_workspace::temp_workspace()
	{
		static std::atomic<unsigned long long> counter{ 0 };
		const auto stamp = std::chrono::steady_clock::now()
			.time_since_epoch().count();
		const std::string leaf = "cppstr_langcvt_" +
			std::to_string(stamp) + "_" +
			std::to_string(counter.fetch_add(1, std::memory_order_relaxed));

		std::error_code error;
		const std::filesystem::path base = std::filesystem::temp_directory_path(error);
		if (!error)
		{
			directory = base / leaf;
			std::filesystem::create_directories(directory, error);
		}
		if (error)
		{
			const cppString message =
				cppString("langcvt: cannot create temporary directory: ") +
				error.message().c_str();
			throw Invalid_Operation(message.s_str());
		}
	}

	temp_workspace::~temp_workspace()
	{
		std::error_code ignored;
		std::filesystem::remove_all(directory, ignored);
	}

	bool is_msvc_driver(const cppString& driver)
	{
		std::string name = driver.s_str();
		for (char& c : name)
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

		const std::string stem = std::filesystem::path(name).stem().string();
		return stem == "cl" || stem == "clang-cl";
	}

	cppString native_path(const std::filesystem::path& path)
	{
		std::filesystem::path preferred = path;
		preferred.make_preferred();
		return cppString{ preferred.string() };
	}

	Result compile_and_run(
		const cppString& source,
		const cppString& source_name,
		const std::function<std::vector<cppString>(
			const std::filesystem::path&,
			const std::filesystem::path&)>& plan,
		const std::vector<cppString>& run_arguments,
		const CommonOptions& options,
		const char* module_name,
		const cppString& runner)
	{
		if (source.empty())
			throw Invalid_Argument(
				(cppString(module_name) + ": source code must not be empty.")
					.s_str());

		temp_workspace workspace;
		const std::filesystem::path source_path =
			workspace.directory / source_name.s_str();
		std::string stem =
			std::filesystem::path(source_name.s_str()).stem().string();
#ifdef _WIN32
		stem += ".exe";
#endif
		const std::filesystem::path executable_path = workspace.directory / stem;

		{
			std::ofstream stream(source_path, std::ios::binary);
			if (!stream)
				throw Invalid_Operation(
					(cppString(module_name) +
						": cannot create temporary source file.").s_str());
			stream << source.s_str();
			if (!stream)
				throw Invalid_Operation(
					(cppString(module_name) +
						": cannot write temporary source file.").s_str());
		}

		const std::vector<cppString> compile_argv =
			plan(source_path, executable_path);
		if (compile_argv.empty() || compile_argv[0].empty())
			throw Invalid_Argument(
				(cppString(module_name) +
					": compile command must not be empty.").s_str());

		// Compile step: diagnostics go to their own streams so they can be
		// reported verbatim; the run options apply to the execution step.
		CommonOptions compile_options;
		compile_options.working_directory = options.working_directory;
		compile_options.timeout = options.timeout;
		compile_options.merge_stderr = false;
		const Result compiled = run_argv(compile_argv, compile_options);
		if (!compiled.ok())
			return compiled;

		std::vector<cppString> argv;
		if (!runner.empty())
			argv.push_back(runner);
		argv.push_back(native_path(executable_path));
		for (const cppString& argument : run_arguments)
			argv.push_back(argument);
		return run_argv(argv, options);
	}
}
