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

#include <langcvt/Javacvt.h>
#include <langcvt/langcvt.h>
#include <atomic>
#include <chrono>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace
{
	cppstr::cppString detect_class_name(const cppstr::cppString& source)
	{
		const std::string& text = source.s_str();

		auto name_after = [&text](const std::string& token) -> cppstr::cppString
		{
			const std::size_t at = text.find(token);
			if (at == std::string::npos)
				return cppstr::cppString{};

			std::size_t i = at + token.size();
			while (i < text.size() &&
				std::isspace(static_cast<unsigned char>(text[i])) != 0)
				++i;

			const std::size_t begin = i;
			while (i < text.size() &&
				(std::isalnum(static_cast<unsigned char>(text[i])) != 0 ||
					text[i] == '_' || text[i] == '$'))
				++i;

			if (i == begin)
				return cppstr::cppString{};
			return cppstr::cppString{ text.substr(begin, i - begin) };
		};

		const char* declarations[] = {
			"public class ", "class ",
			"public interface ", "interface ",
			"public enum ", "enum ",
			"public record ", "record "
		};
		for (const char* declaration : declarations)
		{
			cppstr::cppString name = name_after(declaration);
			if (!name.empty())
				return name;
		}
		return cppstr::cppString{};
	}

	std::filesystem::path unique_temp_dir()
	{
		static std::atomic<unsigned long long> counter{ 0 };
		const auto stamp = std::chrono::steady_clock::now()
			.time_since_epoch().count();
		const std::string leaf = "cppstr_javacvt_" +
			std::to_string(stamp) + "_" +
			std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
		return std::filesystem::temp_directory_path() / leaf;
	}

	struct temp_dir_guard
	{
		std::filesystem::path path;

		~temp_dir_guard()
		{
			std::error_code ignored;
			std::filesystem::remove_all(path, ignored);
		}
	};
}

namespace cppstr::javacvt
{
	cppstr::cppString default_runtime()
	{
		const char* candidates[] = { "java" };
		for (const char* candidate : candidates)
		{
			if (langcvt::interpreter_exists(candidate))
				return cppstr::cppString{ candidate };
		}
		return cppstr::cppString{ candidates[0] };
	}

	langcvt::Result run(const cppstr::cppString& source, const Options& options)
	{
		const cppstr::cppString runtime = options.runtime.empty()
			? default_runtime()
			: options.runtime;
		if (runtime.empty())
			throw Invalid_Argument("javacvt: runtime must not be empty.");

		const cppstr::cppString class_name = options.class_name.empty()
			? detect_class_name(source)
			: options.class_name;
		if (class_name.empty())
			throw Invalid_Argument(
				"javacvt: cannot determine class name; set Options::class_name.");

		const std::filesystem::path directory = unique_temp_dir();
		std::error_code error;
		std::filesystem::create_directories(directory, error);
		if (error)
		{
			const cppstr::cppString message =
				cppstr::cppString("javacvt: cannot create temporary directory: ") +
				error.message().c_str();
			throw Invalid_Operation(message.s_str());
		}

		temp_dir_guard guard{ directory };
		const std::filesystem::path source_file =
			directory / (class_name.s_str() + ".java");

		{
			std::ofstream stream(source_file, std::ios::binary);
			if (!stream)
				throw Invalid_Operation("javacvt: cannot create temporary source file.");
			stream << source.s_str();
			if (!stream)
				throw Invalid_Operation("javacvt: cannot write temporary source file.");
		}

		std::vector<cppstr::cppString> argv;
		argv.push_back(runtime);
		argv.push_back(cppstr::cppString{ source_file.generic_string() });
		return langcvt::run_argv(argv, options);
	}

	cppstr::cppString eval(const cppstr::cppString& source, const Options& options)
	{
		return langcvt::eval_result(run(source, options), "Java execution");
	}
}
