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

#include <cppString/langcvt/c#cvt.h>
#include <cppString/langcvt/langcvt.h>

namespace
{
	// The classic .NET Framework compilers are not on PATH; probe the well
	// known installation locations as a fallback (Windows only).
	std::vector<cppstr::cppString> compiler_candidates()
	{
#ifdef _WIN32
		return {
			"csc",
			"C:\\Windows\\Microsoft.NET\\Framework64\\v4.0.30319\\csc.exe",
			"C:\\Windows\\Microsoft.NET\\Framework\\v4.0.30319\\csc.exe",
		};
#else
		return { "mcs", "csc" };
#endif
	}
}

namespace cppstr::csharpcvt
{
	cppstr::cppString default_compiler()
	{
		for (const cppstr::cppString& candidate : compiler_candidates())
		{
			if (langcvt::interpreter_exists(candidate))
				return candidate;
		}
		return cppstr::cppString{ "csc" };
	}

	langcvt::Result run(const cppstr::cppString& source, const Options& options)
	{
		const cppstr::cppString compiler = options.compiler.empty()
			? default_compiler()
			: options.compiler;
		if (compiler.empty())
			throw Invalid_Argument("csharpcvt: compiler must not be empty.");

		const auto plan = [&compiler, &options](
			const std::filesystem::path& source_path,
			const std::filesystem::path& executable_path)
		{
			std::vector<cppstr::cppString> argv;
			argv.push_back(compiler);
			argv.push_back("/nologo");
			for (const cppstr::cppString& argument : options.arguments)
				argv.push_back(argument);
			argv.push_back(cppstr::cppString("/out:") +
				langcvt::native_path(executable_path).s_str().c_str());
			argv.push_back(langcvt::native_path(source_path));
			return argv;
		};

		return langcvt::compile_and_run(source, "Program.cs", plan,
			{}, options, "csharpcvt", options.runner);
	}

	cppstr::cppString eval(const cppstr::cppString& source, const Options& options)
	{
		return langcvt::eval_result(run(source, options), "C# execution");
	}
}
