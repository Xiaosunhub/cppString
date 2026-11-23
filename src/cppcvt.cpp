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

#include <cppString/langcvt/cppcvt.h>
#include <cppString/langcvt/langcvt.h>

namespace cppstr::cppcvt
{
	cppstr::cppString default_compiler()
	{
		const char* candidates[] = { "c++", "g++", "clang++", "cl" };
		for (const char* candidate : candidates)
		{
			if (langcvt::interpreter_exists(candidate))
				return cppstr::cppString{ candidate };
		}
		return cppstr::cppString{ candidates[0] };
	}

	langcvt::Result run(const cppstr::cppString& source, const Options& options)
	{
		const cppstr::cppString compiler = options.compiler.empty()
			? default_compiler()
			: options.compiler;
		if (compiler.empty())
			throw Invalid_Argument("cppcvt: compiler must not be empty.");

		const auto plan = [&compiler, &options](
			const std::filesystem::path& source_path,
			const std::filesystem::path& executable_path)
		{
			std::vector<cppstr::cppString> argv;
			if (langcvt::is_msvc_driver(compiler))
			{
				argv.push_back(compiler);
				argv.push_back("/nologo");
				argv.push_back("/EHsc");
				for (const cppstr::cppString& argument : options.arguments)
					argv.push_back(argument);
				argv.push_back(cppstr::cppString("/Fe") +
					langcvt::native_path(executable_path).s_str().c_str());
				argv.push_back(langcvt::native_path(source_path));
			}
			else
			{
				argv.push_back(compiler);
				for (const cppstr::cppString& argument : options.arguments)
					argv.push_back(argument);
				argv.push_back("-o");
				argv.push_back(langcvt::native_path(executable_path));
				argv.push_back(langcvt::native_path(source_path));
			}
			return argv;
		};

		return langcvt::compile_and_run(source, "main.cpp", plan,
			{}, options, "cppcvt");
	}

	cppstr::cppString eval(const cppstr::cppString& source, const Options& options)
	{
		return langcvt::eval_result(run(source, options), "C++ execution");
	}
}
