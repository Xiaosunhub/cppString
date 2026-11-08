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

#include "file.h"
#include "checksum.h"
#include "method.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace fs = std::filesystem;

namespace
{
	// std::filesystem::path uses UTF-8 char8_t sources/outputs on every C++20
	// implementation, so non-ASCII paths survive the round trip on Windows.
	fs::path to_path(const sp::cppString& value)
	{
		const ::String& text = value.s_str();
		return fs::path{ std::u8string_view{
			reinterpret_cast<const char8_t*>(text.data()), text.size() } };
	}

	sp::cppString from_path(const fs::path& value)
	{
		const std::u8string text = value.u8string();
		return sp::cppString{ ::String{
			reinterpret_cast<const char*>(text.data()), text.size() } };
	}

	// Preserve the library's typed exceptions; map everything else coming out
	// of std::filesystem to Invalid_Operation.
	template<class F>
	decltype(auto) guarded(const char* message, F&& fn)
	{
		try
		{
			return fn();
		}
		catch (const sp::String_Error&)
		{
			throw;
		}
		catch (...)
		{
			throw sp::Invalid_Operation(message);
		}
	}

	::String read_all(const sp::cppString& path)
	{
		std::ifstream input{ to_path(path), std::ios::binary };
		if (!input)
			throw sp::Invalid_Operation("sp::file: cannot open file for reading");

		::String data{ std::istreambuf_iterator<char>{ input }, std::istreambuf_iterator<char>{} };
		if (input.bad())
			throw sp::Invalid_Operation("sp::file: read failed");
		return data;
	}

	void write_all(const sp::cppString& path, const ::String& bytes, bool append)
	{
		const auto mode = std::ios::binary | (append ? std::ios::app : std::ios::trunc);
		std::ofstream output{ to_path(path), mode };
		if (!output)
			throw sp::Invalid_Operation("sp::file: cannot open file for writing");

		if (!bytes.empty())
			output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
		output.flush();

		if (!output)
			throw sp::Invalid_Operation("sp::file: write failed");
	}
}

namespace sp
{
	namespace file
	{
		// ---- queries ---------------------------------------------------------

		bool exists(const cppString& path) noexcept
		{
			try { std::error_code ec; return fs::exists(to_path(path), ec) && !ec; }
			catch (...) { return false; }
		}

		bool is_regular_file(const cppString& path) noexcept
		{
			try { std::error_code ec; return fs::is_regular_file(to_path(path), ec) && !ec; }
			catch (...) { return false; }
		}

		bool is_directory(const cppString& path) noexcept
		{
			try { std::error_code ec; return fs::is_directory(to_path(path), ec) && !ec; }
			catch (...) { return false; }
		}

		bool is_empty(const cppString& path) noexcept
		{
			try { std::error_code ec; return fs::is_empty(to_path(path), ec) && !ec; }
			catch (...) { return false; }
		}

		bool equivalent(const cppString& lhs, const cppString& rhs) noexcept
		{
			try { std::error_code ec; return fs::equivalent(to_path(lhs), to_path(rhs), ec) && !ec; }
			catch (...) { return false; }
		}

		bool equals(const cppString& lhs, const cppString& rhs)
		{
			return guarded("sp::file::equals: cannot compare files", [&]() -> bool
				{
					if (sp::file::size(lhs) != sp::file::size(rhs))
						return false;

					std::ifstream a{ to_path(lhs), std::ios::binary };
					std::ifstream b{ to_path(rhs), std::ios::binary };
					if (!a || !b)
						throw sp::Invalid_Operation("sp::file::equals: cannot open file");

					std::array<char, 65536> left{};
					std::array<char, 65536> right{};

					while (true)
					{
						a.read(left.data(), static_cast<std::streamsize>(left.size()));
						b.read(right.data(), static_cast<std::streamsize>(right.size()));
						const auto na = a.gcount();
						const auto nb = b.gcount();
						if (na != nb)
							return false;
						if (na > 0 && std::memcmp(left.data(), right.data(),
							static_cast<std::size_t>(na)) != 0)
							return false;
						if (na < static_cast<std::streamsize>(left.size()))
							break;
					}

					return true;
				});
		}

		// ---- metadata & path components -------------------------------------

		std::uintmax_t size(const cppString& path)
		{
			return guarded("sp::file::size: cannot stat file", [&]() -> std::uintmax_t
				{
					std::error_code ec;
					const auto value = fs::file_size(to_path(path), ec);
					if (ec)
						throw sp::Invalid_Operation("sp::file::size: cannot stat file");
					return value;
				});
		}

		std::int64_t last_modified(const cppString& path)
		{
			return guarded("sp::file::last_modified: cannot read timestamp", [&]() -> std::int64_t
				{
					std::error_code ec;
					const auto stamp = fs::last_write_time(to_path(path), ec);
					if (ec)
						throw sp::Invalid_Operation("sp::file::last_modified: cannot read timestamp");

					// Convert file_clock to system_clock without relying on an
					// implementation-specific epoch offset.
					const auto system_stamp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
						stamp - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
					return static_cast<std::int64_t>(
						std::chrono::duration_cast<std::chrono::seconds>(
							system_stamp.time_since_epoch()).count());
				});
		}

		cppString size_string(const cppString& path)
		{
			return sp::human_size(static_cast<unsigned long long>(sp::file::size(path)));
		}

		cppString filename(const cppString& path)
		{
			return guarded("sp::file::filename: bad path", [&]() { return from_path(to_path(path).filename()); });
		}

		cppString stem(const cppString& path)
		{
			return guarded("sp::file::stem: bad path", [&]() { return from_path(to_path(path).stem()); });
		}

		cppString extension(const cppString& path)
		{
			return guarded("sp::file::extension: bad path", [&]() { return from_path(to_path(path).extension()); });
		}

		// ---- path resolution / working directory ----------------------------

		cppString absolute(const cppString& path)
		{
			return guarded("sp::file::absolute: cannot resolve path", [&]() { return from_path(fs::absolute(to_path(path))); });
		}

		cppString canonical(const cppString& path)
		{
			return guarded("sp::file::canonical: path does not exist", [&]() { return from_path(fs::canonical(to_path(path))); });
		}

		cppString weakly_canonical(const cppString& path)
		{
			return guarded("sp::file::weakly_canonical: cannot resolve path", [&]() { return from_path(fs::weakly_canonical(to_path(path))); });
		}

		cppString relative(const cppString& path, const cppString& base)
		{
			return guarded("sp::file::relative: cannot resolve path", [&]() { return from_path(fs::relative(to_path(path), to_path(base))); });
		}

		cppString current_path()
		{
			return guarded("sp::file::current_path: cannot query cwd", []() { return from_path(fs::current_path()); });
		}

		void set_current_path(const cppString& path)
		{
			guarded("sp::file::set_current_path: cannot change cwd", [&]() { fs::current_path(to_path(path)); });
		}

		cppString temp_directory()
		{
			return guarded("sp::file::temp_directory: cannot query temp path", []() { return from_path(fs::temp_directory_path()); });
		}

		cppString home_directory()
		{
			const char* value = std::getenv("USERPROFILE");
			if (value == nullptr)
				value = std::getenv("HOME");
			return value != nullptr ? cppString{ ::String{ value } } : cppString{};
		}

		// ---- lifecycle -------------------------------------------------------

		bool create_directory(const cppString& path) noexcept
		{
			try { return fs::create_directory(to_path(path)); }
			catch (...) { return false; }
		}

		bool create_directories(const cppString& path) noexcept
		{
			try { return fs::create_directories(to_path(path)); }
			catch (...) { return false; }
		}

		bool remove(const cppString& path) noexcept
		{
			try { std::error_code ec; return fs::remove(to_path(path), ec) && !ec; }
			catch (...) { return false; }
		}

		std::uintmax_t remove_all(const cppString& path) noexcept
		{
			try
			{
				std::error_code ec;
				const auto count = fs::remove_all(to_path(path), ec);
				return ec ? 0 : count;
			}
			catch (...) { return 0; }
		}

		bool copy(const cppString& from, const cppString& to, bool overwrite) noexcept
		{
			try
			{
				const auto source = to_path(from);
				const auto target = to_path(to);

				// Never let the "overwrite" cleanup delete the source itself.
				std::error_code same;
				if (fs::equivalent(source, target, same) && !same)
					return false;

				if (!fs::exists(source))
					return false;
				if (fs::exists(target))
				{
					if (!overwrite)
						return false;
					std::error_code ec;
					fs::remove_all(target, ec);
					if (ec)
						return false;
				}
				fs::copy(source, target,
					fs::copy_options::recursive | fs::copy_options::overwrite_existing);
				return fs::exists(target);
			}
			catch (...) { return false; }
		}

		bool move(const cppString& from, const cppString& to, bool overwrite) noexcept
		{
			try
			{
				const auto source = to_path(from);
				const auto target = to_path(to);

				// Renaming a path onto itself is a no-op; with overwrite=true
				// the old code removed the source before renaming it.
				std::error_code same;
				if (fs::equivalent(source, target, same) && !same)
					return false;

				if (!fs::exists(source))
					return false;
				if (fs::exists(target))
				{
					if (!overwrite)
						return false;
					std::error_code ec;
					fs::remove_all(target, ec);
					if (ec)
						return false;
				}
				std::error_code ec;
				fs::rename(source, target, ec);
				return !ec;
			}
			catch (...) { return false; }
		}

		bool touch(const cppString& path) noexcept
		{
			try
			{
				const auto target = to_path(path);
				if (fs::exists(target))
				{
					std::error_code ec;
					fs::last_write_time(target, fs::file_time_type::clock::now(), ec);
					return !ec;
				}
				std::ofstream output{ target, std::ios::binary | std::ios::app };
				return output.good();
			}
			catch (...) { return false; }
		}

		void truncate(const cppString& path, std::uintmax_t new_size)
		{
			guarded("sp::file::truncate: cannot resize file", [&]()
				{
					std::error_code ec;
					fs::resize_file(to_path(path), new_size, ec);
					if (ec)
						throw sp::Invalid_Operation("sp::file::truncate: cannot resize file");
				});
		}

		// ---- content ---------------------------------------------------------

		cppString read_text(const cppString& path)
		{
			return cppString{ read_all(path) };
		}

		void write_text(const cppString& path, const cppString& text, bool append)
		{
			write_all(path, text.s_str(), append);
		}

		void append_text(const cppString& path, const cppString& text)
		{
			write_text(path, text, true);
		}

		std::vector<cppString> read_lines(const cppString& path, bool keep_empty)
		{
			const ::String data = read_all(path);
			std::vector<cppString> lines;
			std::size_t cursor = 0;

			while (cursor < data.size())
			{
				const std::size_t hit = data.find('\n', cursor);
				::String line = hit == ::String::npos
					? data.substr(cursor)
					: data.substr(cursor, hit - cursor);

				if (!line.empty() && line.back() == '\r')
					line.pop_back();
				if (keep_empty || !line.empty())
					lines.push_back(cppString{ std::move(line) });

				if (hit == ::String::npos)
					break;
				cursor = hit + 1;
			}

			return lines;
		}

		void write_lines(const cppString& path, const std::vector<cppString>& lines,
			const cppString& newline)
		{
			const ::String separator = newline.s_str();
			::String buffer;
			for (const auto& line : lines)
			{
				buffer += line.s_str();
				buffer += separator;
			}
			write_all(path, buffer, false);
		}

		String read_bytes(const cppString& path)
		{
			return read_all(path);
		}

		void write_bytes(const cppString& path, const String& bytes, bool append)
		{
			write_all(path, bytes, append);
		}

		String read_chunk(const cppString& path, std::uintmax_t offset, std::size_t length)
		{
			return guarded("sp::file::read_chunk: cannot read file", [&]() -> String
				{
					std::ifstream input{ to_path(path), std::ios::binary };
					if (!input)
						throw sp::Invalid_Operation("sp::file::read_chunk: cannot open file");

					input.seekg(0, std::ios::end);
					const auto end = input.tellg();
					if (end < 0)
						throw sp::Invalid_Operation("sp::file::read_chunk: cannot determine file size");
					const auto total = static_cast<std::uintmax_t>(end);
					if (offset > total)
						throw sp::Index_Out_Of_Range("sp::file::read_chunk: offset past end of file");

					const auto available = total - offset;
					const std::size_t count = static_cast<std::size_t>(
						std::min<std::uintmax_t>(length, available));
					String buffer(count, '\0');
					if (count == 0)
						return buffer;

					input.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
					input.read(buffer.data(), static_cast<std::streamsize>(count));
					buffer.resize(static_cast<std::size_t>(input.gcount()));
					return buffer;
				});
		}

		std::size_t for_each_line(const cppString& path,
			const std::function<bool(const cppString&, std::size_t)>& visitor)
		{
			if (!visitor)
				throw sp::Invalid_Argument("sp::file::for_each_line: visitor must not be empty");

			const auto lines = read_lines(path, true);
			std::size_t visited = 0;
			for (; visited < lines.size(); ++visited)
			{
				if (!visitor(lines[visited], visited))
				{
					++visited;
					break;
				}
			}
			return visited;
		}

		void write_text_atomic(const cppString& path, const cppString& text)
		{
			const cppString temporary = path + cppString{ ".tmp" };
			try
			{
				write_all(temporary, text.s_str(), false);
				const auto source = to_path(temporary);
				const auto target = to_path(path);

#ifdef _WIN32
				// std::filesystem::rename cannot replace an existing file on
				// Windows. Deleting the destination first leaves a window where
				// the file is missing, and a failed rename permanently loses the
				// original content. MoveFileExW swaps in one atomic step.
				if (!MoveFileExW(source.c_str(), target.c_str(),
					MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
					throw sp::Invalid_Operation("sp::file::write_text_atomic: rename failed");
#else
				std::error_code ec;
				fs::rename(source, target, ec);
				if (ec)
					throw sp::Invalid_Operation("sp::file::write_text_atomic: rename failed");
#endif
			}
			catch (...)
			{
				std::error_code ignored;
				fs::remove(to_path(temporary), ignored);
				throw;
			}
		}

		// ---- directory listing ----------------------------------------------

		std::vector<Entry> list(const cppString& directory, bool recursive)
		{
			return guarded("sp::file::list: cannot read directory", [&]() -> std::vector<Entry>
				{
					const auto root = to_path(directory);
					if (!fs::is_directory(root))
						throw sp::Invalid_Operation("sp::file::list: not a directory");

					std::vector<Entry> entries;
					auto add = [&](const fs::directory_entry& item)
						{
							Entry entry;
							entry.path = from_path(item.path());
							entry.name = from_path(item.path().filename());
							entry.is_directory = item.is_directory();
							entry.is_regular = item.is_regular_file();
							if (entry.is_regular)
								entry.size = item.file_size();
							entries.push_back(std::move(entry));
						};

					if (recursive)
					{
						for (const auto& item : fs::recursive_directory_iterator{ root })
							add(item);
					}
					else
					{
						for (const auto& item : fs::directory_iterator{ root })
							add(item);
					}

					std::sort(entries.begin(), entries.end(),
						[](const Entry& a, const Entry& b) { return a.path.s_str() < b.path.s_str(); });
					return entries;
				});
		}

		std::vector<cppString> find(const cppString& directory, const cppString& extension,
			bool recursive)
		{
			cppString wanted = extension;
			if (!wanted.empty() && !wanted.startsWith("."))
				wanted = cppString{ "." } + wanted;

			const auto entries = sp::file::list(directory, recursive);
			std::vector<cppString> matches;
			for (const auto& entry : entries)
			{
				if (!entry.is_regular)
					continue;
				if (wanted.empty() || sp::file::extension(entry.path) == wanted)
					matches.push_back(entry.path);
			}
			return matches;
		}

		// ---- hashing ---------------------------------------------------------

		cppString hash(const cppString& path, const cppString& algorithm)
		{
			const cppString name = algorithm.copyLower();

			// Validate before touching the filesystem: an unknown algorithm
			// must report Invalid_Argument even when the file does not exist.
			if (name != "md5" && name != "sha256" && name != "crc32"
				&& name != "adler32" && name != "fnv1a64")
				throw sp::Invalid_Argument("sp::file::hash: unknown algorithm");

			const cppString data{ read_bytes(path) };

			if (name == "md5")      return sp::checksum::md5_hex(data);
			if (name == "sha256")   return sp::checksum::sha256_hex(data);
			if (name == "crc32")    return sp::checksum::crc32_hex(data);
			if (name == "adler32")  return sp::checksum::adler32_hex(data);
			return sp::checksum::fnv1a64_hex(data);
		}
	}
}
