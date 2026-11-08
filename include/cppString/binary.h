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

#ifndef CPPSTRING_BINARY_H_
#define CPPSTRING_BINARY_H_

#include "cppString.h"
#include <cstdint>
#include <optional>
#include <type_traits>
#include <vector>

namespace sp
{
	// Raw byte helpers. A byte buffer is a `sp::String` (std::string): it may
	// contain embedded NUL bytes, so no C string assumption is ever made.
	// Textual representations use `cppString`. Reads that run past the end of
	// a buffer throw Index_Out_Of_Range; malformed text throws
	// Invalid_Conversion; bad parameters throw Invalid_Argument.
	namespace binary
	{
		enum class Endian
		{
			Little,
			Big
		};

		// Byte order of the host running the library.
		Endian native_endian() noexcept;

		// ---- text <-> bytes -------------------------------------------------

		// Two hex digits per byte. `separator` is inserted between bytes only.
		cppString to_hex(const String& bytes, bool uppercase = false,
			const cppString& separator = cppString{});

		// Accepts the usual dumps: spaces, colons, dashes, commas, underscores,
		// newlines and `0x` prefixes are ignored. An odd number of digits or
		// any other character throws Invalid_Conversion.
		String from_hex(const cppString& text);

		// Eight bits per byte, MSB first (default separator is one space).
		cppString to_bits(const String& bytes, const cppString& separator = cppString{ " " });
		String from_bits(const cppString& text);

		// Classic hex dump: "00000000  48 65 6c 6c 6f ...  |Hello|".
		// Every line is padded to `bytes_per_line` columns and ends with '\n'.
		// bytes_per_line == 0 throws Invalid_Argument.
		cppString hex_dump(const String& bytes, std::size_t bytes_per_line = 16,
			bool show_ascii = true);

		// Replace non-printable bytes (outside 0x20..0x7E and \\t\\n\\r) with
		// `placeholder`; is_printable reports whether any replacement is needed.
		cppString printable(const String& bytes, char placeholder = '.');
		bool is_printable(const String& bytes) noexcept;

		// ---- bit-level operations -------------------------------------------

		// Bit index 0 is the least significant bit of byte 0.
		bool get_bit(const String& bytes, std::size_t bit_index);
		void set_bit(String& bytes, std::size_t bit_index, bool value);
		std::size_t popcount(const String& bytes) noexcept;
		std::uint8_t reverse_bits(std::uint8_t byte) noexcept;

		String bit_not(const String& bytes);
		String bit_and(const String& lhs, const String& rhs);
		String bit_or(const String& lhs, const String& rhs);
		String bit_xor(const String& lhs, const String& rhs);
		// The three binary operators throw Invalid_Argument on a length mismatch.

		std::uint16_t byteswap16(std::uint16_t value) noexcept;
		std::uint32_t byteswap32(std::uint32_t value) noexcept;
		std::uint64_t byteswap64(std::uint64_t value) noexcept;
		String reverse_bytes(const String& bytes);

		// ---- buffer scanning -------------------------------------------------

		std::size_t find(const String& data, const String& pattern,
			std::size_t from = 0) noexcept;
		std::vector<std::size_t> find_all(const String& data, const String& pattern);
		std::vector<String> split(const String& data, const String& delimiter);
		String slice(const String& data, std::size_t offset,
			std::size_t length = static_cast<std::size_t>(-1));

		// ---- raw file I/O (binary mode, no text translation) -----------------

		String load_file(const cppString& path);            // throws Invalid_Operation
		void save_file(const cppString& path, const String& bytes); // throws Invalid_Operation

		// ---- cursor-based parser --------------------------------------------

		// All multi-byte reads honour the reader's default byte order; pass an
		// explicit Endian to override it for a single call.
		class CPPSTRING_EXPORT Reader
		{
		public:
			explicit Reader(String bytes, Endian endian = Endian::Little);
			static Reader from_file(const cppString& path, Endian endian = Endian::Little);
			static Reader from_hex(const cppString& text, Endian endian = Endian::Little);

			const String& data() const noexcept;
			std::size_t size() const noexcept;
			std::size_t tell() const noexcept;
			std::size_t remaining() const noexcept;
			bool eof() const noexcept;

			Endian endian() const noexcept;
			void set_endian(Endian value) noexcept;
			void seek(std::size_t position);       // position may equal size()
			void skip(std::size_t count);
			void align(std::size_t boundary);      // advance to a multiple of boundary
			void rewind() noexcept;

			std::uint8_t  u8();
			std::int8_t   i8();
			std::uint16_t u16(std::optional<Endian> order = std::nullopt);
			std::int16_t  i16(std::optional<Endian> order = std::nullopt);
			std::uint32_t u32(std::optional<Endian> order = std::nullopt);
			std::int32_t  i32(std::optional<Endian> order = std::nullopt);
			std::uint64_t u64(std::optional<Endian> order = std::nullopt);
			std::int64_t  i64(std::optional<Endian> order = std::nullopt);
			float         f32(std::optional<Endian> order = std::nullopt);
			double        f64(std::optional<Endian> order = std::nullopt);

			String bytes(std::size_t count);        // consume exactly count bytes
			String peek(std::size_t count) const;  // non-consuming
			String rest();                         // consume everything left
			String read_until(std::uint8_t delimiter, bool consume_delimiter = true);
			String cstring();                      // up to (and consuming) a NUL byte
			String line();                         // up to '\n', trailing '\r' removed

			// Unsigned LEB128 and its zig-zag encoded signed counterpart.
			std::uint64_t varint();
			std::int64_t  svarint();

			// Signature helpers. match() only advances on success; skip_to()
			// moves the cursor to the next occurrence and reports whether it
			// was found. find() returns an absolute offset or String::npos.
			bool match(const String& signature, bool consume = true);
			bool skip_to(const String& signature, bool consume = false);
			std::size_t find(const String& signature, std::size_t from = 0) const noexcept;

		private:
			void require(std::size_t count) const;
			Endian effective(std::optional<Endian> order) const noexcept;
			std::uint64_t read_uint(std::size_t width, Endian order);

			String m_data{};
			std::size_t m_pos = 0;
			Endian m_endian = Endian::Little;
		};

		// ---- cursor-based builder -------------------------------------------

		// Symmetric counterpart of Reader; every write method returns *this so
		// calls can be chained: w.u16(0x1234).cstring("hi").varint(300).
		class CPPSTRING_EXPORT Writer
		{
		public:
			explicit Writer(Endian endian = Endian::Little);

			void reserve(std::size_t capacity);
			void clear() noexcept;
			std::size_t size() const noexcept;
			const String& data() const noexcept;
			String take();                      // move the buffer out and reset
			cppString to_hex() const;

			Endian endian() const noexcept;
			void set_endian(Endian value) noexcept;
			void align(std::size_t boundary, std::uint8_t fill = 0);

			Writer& u8(std::uint8_t value);
			Writer& i8(std::int8_t value);
			Writer& u16(std::uint16_t value, std::optional<Endian> order = std::nullopt);
			Writer& i16(std::int16_t value, std::optional<Endian> order = std::nullopt);
			Writer& u32(std::uint32_t value, std::optional<Endian> order = std::nullopt);
			Writer& i32(std::int32_t value, std::optional<Endian> order = std::nullopt);
			Writer& u64(std::uint64_t value, std::optional<Endian> order = std::nullopt);
			Writer& i64(std::int64_t value, std::optional<Endian> order = std::nullopt);
			Writer& f32(float value, std::optional<Endian> order = std::nullopt);
			Writer& f64(double value, std::optional<Endian> order = std::nullopt);

			Writer& bytes(const String& value);
			Writer& raw(const void* value, std::size_t count);
			Writer& cstring(const String& value);   // value + trailing NUL
			Writer& varint(std::uint64_t value);
			Writer& svarint(std::int64_t value);
			Writer& pad(std::size_t count, std::uint8_t fill = 0);

		private:
			Endian effective(std::optional<Endian> order) const noexcept;
			void write_uint(std::uint64_t value, std::size_t width, Endian order);

			String m_data{};
			Endian m_endian = Endian::Little;
		};

		// A convenient alias: pack a little/big endian integer into raw bytes.
		template<typename T>
		String pack(T value, Endian order = Endian::Little)
		{
			static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
				"binary::pack requires an arithmetic type");

			Writer w{ order };
			using U = std::remove_cv_t<T>;

			if constexpr (std::is_same_v<U, std::uint8_t>) w.u8(value);
			else if constexpr (std::is_same_v<U, std::int8_t>) w.i8(value);
			else if constexpr (std::is_same_v<U, std::uint16_t>) w.u16(value);
			else if constexpr (std::is_same_v<U, std::int16_t>) w.i16(value);
			else if constexpr (std::is_same_v<U, std::uint32_t>) w.u32(value);
			else if constexpr (std::is_same_v<U, std::int32_t>) w.i32(value);
			else if constexpr (std::is_same_v<U, std::uint64_t>) w.u64(value);
			else if constexpr (std::is_same_v<U, std::int64_t>) w.i64(value);
			else if constexpr (std::is_same_v<U, float>) w.f32(value);
			else if constexpr (std::is_same_v<U, double>) w.f64(value);
			else if constexpr (std::is_same_v<U, bool>) w.u8(value ? 1 : 0);
			else if constexpr (std::is_signed_v<U>) w.i32(static_cast<std::int32_t>(value));
			else w.u32(static_cast<std::uint32_t>(value));

			return w.take();
		}
	}
}

#endif
