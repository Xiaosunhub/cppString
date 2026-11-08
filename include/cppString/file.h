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

#ifndef CPPSTRING_FILE_H_
#define CPPSTRING_FILE_H_

#include "cppString.h"
#include <cstdint>
#include <functional>
#include <vector>

namespace sp
{
	// Filesystem operations built on std::filesystem. Paths are UTF-8 strings.
	// This module is deliberately broader than the small read_file/write_file
	// helpers in method.h (plain text I/O) and load_file/save_file in binary.h
	// (raw byte buffers): it adds metadata, path resolution, directories,
	// lifecycle operations, streaming-friendly reads and file hashing.
	//
	// Query functions never throw. Operations that can fail on the filesystem
	// report failure with a typed sp::Invalid_Operation (or return false for
	// the bool-returning lifecycle helpers). Bad offsets throw
	// Index_Out_Of_Range and unknown algorithms throw Invalid_Argument.
	namespace file
	{
		// One directory listing item. `path` is the full path, `name` is the
		// final component; sizes are 0 for directories.
		struct CPPSTRING_EXPORT Entry
		{
			cppString path{};
			cppString name{};
			std::uintmax_t size = 0;
			bool is_directory = false;
			bool is_regular = false;
		};

		// ---- queries (never throw) ------------------------------------------

		bool exists(const cppString& path) noexcept;
		bool is_regular_file(const cppString& path) noexcept;
		bool is_directory(const cppString& path) noexcept;
		bool is_empty(const cppString& path) noexcept;
		bool equivalent(const cppString& lhs, const cppString& rhs) noexcept;
		bool equals(const cppString& lhs, const cppString& rhs); // byte compare

		// ---- metadata & path components -------------------------------------

		std::uintmax_t size(const cppString& path);
		std::int64_t last_modified(const cppString& path);   // Unix seconds
		cppString size_string(const cppString& path);         // e.g. "1.5 KB"
		cppString filename(const cppString& path);
		cppString stem(const cppString& path);
		cppString extension(const cppString& path);           // includes the dot

		// ---- path resolution / working directory ----------------------------

		cppString absolute(const cppString& path);
		cppString canonical(const cppString& path);           // requires existence
		cppString weakly_canonical(const cppString& path);
		cppString relative(const cppString& path, const cppString& base);
		cppString current_path();
		void set_current_path(const cppString& path);
		cppString temp_directory();
		cppString home_directory();

		// ---- lifecycle (bool = success, never throw) ------------------------

		bool create_directory(const cppString& path) noexcept;
		bool create_directories(const cppString& path) noexcept;
		bool remove(const cppString& path) noexcept;          // files / empty dirs
		std::uintmax_t remove_all(const cppString& path) noexcept;
		bool copy(const cppString& from, const cppString& to, bool overwrite = false) noexcept;
		bool move(const cppString& from, const cppString& to, bool overwrite = false) noexcept;
		bool touch(const cppString& path) noexcept;           // create or bump mtime
		void truncate(const cppString& path, std::uintmax_t new_size);

		// ---- content --------------------------------------------------------

		cppString read_text(const cppString& path);
		void write_text(const cppString& path, const cppString& text, bool append = false);
		void append_text(const cppString& path, const cppString& text);
		std::vector<cppString> read_lines(const cppString& path, bool keep_empty = true);
		void write_lines(const cppString& path, const std::vector<cppString>& lines,
			const cppString& newline = cppString{ "\n" });

		String read_bytes(const cppString& path);
		void write_bytes(const cppString& path, const String& bytes, bool append = false);
		String read_chunk(const cppString& path, std::uintmax_t offset, std::size_t length);

		// Visit lines in order; the visitor receives (line, zero-based index).
		// Returning false stops early. Returns the number of lines visited.
		std::size_t for_each_line(const cppString& path,
			const std::function<bool(const cppString&, std::size_t)>& visitor);

		// Write to a sibling "<path>.tmp", then rename over the destination.
		void write_text_atomic(const cppString& path, const cppString& text);

		// ---- directory listing ---------------------------------------------

		// Sorted by path for deterministic results.
		std::vector<Entry> list(const cppString& directory, bool recursive = false);
		// `extension` may be "txt" or ".txt"; empty matches every file.
		std::vector<cppString> find(const cppString& directory,
			const cppString& extension, bool recursive = true);

		// ---- hashing ---------------------------------------------------------

		// Returns lowercase hex for "md5", "sha256", "crc32", "adler32" and
		// "fnv1a64" (case-insensitive). Unknown names throw Invalid_Argument.
		cppString hash(const cppString& path, const cppString& algorithm);
	}
}

#endif
