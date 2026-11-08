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

#include "binary.h"
#include <algorithm>
#include <bit>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>

namespace
{
	constexpr char HEX_LOWER[] = "0123456789abcdef";
	constexpr char HEX_UPPER[] = "0123456789ABCDEF";

	// Narrow std::string paths are interpreted in the active ANSI code page on
	// Windows, which breaks non-ASCII file names. Build a wide UTF-8 aware
	// filesystem path instead (same convention as sp::file).
	std::filesystem::path to_path(const sp::cppString& value)
	{
		const ::String& text = value.s_str();
		return std::filesystem::path{ std::u8string_view{
			reinterpret_cast<const char8_t*>(text.data()), text.size() } };
	}

	unsigned char byte_at(const ::String& s, std::size_t i) noexcept
	{
		return static_cast<unsigned char>(s[i]);
	}

	int hex_value(unsigned char c) noexcept
	{
		if (c >= '0' && c <= '9') return static_cast<int>(c - '0');
		if (c >= 'a' && c <= 'f') return static_cast<int>(c - 'a') + 10;
		if (c >= 'A' && c <= 'F') return static_cast<int>(c - 'A') + 10;
		return -1;
	}

	bool is_hex_separator(unsigned char c) noexcept
	{
		switch (c)
		{
		case ' ': case '\t': case '\n': case '\r':
		case ':': case '-': case ',': case '_':
			return true;
		default:
			return false;
		}
	}

	bool is_bit_separator(unsigned char c) noexcept
	{
		switch (c)
		{
		case ' ': case '\t': case '\n': case '\r':
		case ':': case '-': case '_':
			return true;
		default:
			return false;
		}
	}

	bool is_printable_byte(unsigned char c) noexcept
	{
		return (c >= 0x20 && c <= 0x7E) || c == '\t' || c == '\n' || c == '\r';
	}

	char nibble_hex(unsigned char nibble, bool uppercase) noexcept
	{
		return uppercase ? HEX_UPPER[nibble & 0x0F] : HEX_LOWER[nibble & 0x0F];
	}

	::String bit_op(const ::String& lhs, const ::String& rhs, char op)
	{
		if (lhs.size() != rhs.size())
			throw sp::Invalid_Argument("binary: bit operands must have equal length");

		::String out(lhs.size(), '\0');

		for (std::size_t i = 0; i < lhs.size(); ++i)
		{
			const auto a = byte_at(lhs, i);
			const auto b = byte_at(rhs, i);
			unsigned char v = 0;

			switch (op)
			{
			case '&': v = static_cast<unsigned char>(a & b); break;
			case '|': v = static_cast<unsigned char>(a | b); break;
			default:  v = static_cast<unsigned char>(a ^ b); break;
			}

			out[i] = static_cast<char>(v);
		}

		return out;
	}
}

namespace sp
{
	namespace binary
	{
		Endian native_endian() noexcept
		{
			return std::endian::native == std::endian::little ? Endian::Little : Endian::Big;
		}

		// ---- text <-> bytes -------------------------------------------------

		cppString to_hex(const String& bytes, bool uppercase, const cppString& separator)
		{
			const String sep = separator.s_str();
			String out;
			out.reserve(bytes.size() * (2 + sep.size()));

			for (std::size_t i = 0; i < bytes.size(); ++i)
			{
				if (i != 0) out += sep;
				const auto b = byte_at(bytes, i);
				out += nibble_hex(static_cast<unsigned char>(b >> 4), uppercase);
				out += nibble_hex(b, uppercase);
			}

			return cppString{ std::move(out) };
		}

		String from_hex(const cppString& text)
		{
			const String& src = text.s_str();
			std::vector<unsigned char> digits;
			digits.reserve(src.size());

			for (std::size_t i = 0; i < src.size(); ++i)
			{
				const auto c = byte_at(src, i);

				if (is_hex_separator(c))
					continue;

				// A "0x" / "0X" prefix is skipped as a unit.
				if (c == '0' && i + 1 < src.size()
					&& (src[i + 1] == 'x' || src[i + 1] == 'X'))
				{
					++i;
					continue;
				}

				const int v = hex_value(c);
				if (v < 0)
					throw sp::Invalid_Conversion("binary::from_hex: invalid hexadecimal character");
				digits.push_back(static_cast<unsigned char>(v));
			}

			if (digits.size() % 2 != 0)
				throw sp::Invalid_Conversion("binary::from_hex: odd number of hexadecimal digits");

			String out(digits.size() / 2, '\0');
			for (std::size_t i = 0; i < out.size(); ++i)
				out[i] = static_cast<char>(static_cast<unsigned char>((digits[2 * i] << 4) | digits[2 * i + 1]));

			return out;
		}

		cppString to_bits(const String& bytes, const cppString& separator)
		{
			const String sep = separator.s_str();
			String out;
			out.reserve(bytes.size() * (8 + sep.size()));

			for (std::size_t i = 0; i < bytes.size(); ++i)
			{
				if (i != 0) out += sep;
				const auto b = byte_at(bytes, i);
				for (int bit = 7; bit >= 0; --bit)
					out += ((b >> bit) & 1u) != 0 ? '1' : '0';
			}

			return cppString{ std::move(out) };
		}

		String from_bits(const cppString& text)
		{
			const String& src = text.s_str();
			std::vector<unsigned char> bits;
			bits.reserve(src.size());

			for (std::size_t i = 0; i < src.size(); ++i)
			{
				const auto c = byte_at(src, i);
				if (is_bit_separator(c))
					continue;
				if (c != '0' && c != '1')
					throw sp::Invalid_Conversion("binary::from_bits: only 0 and 1 are allowed");
				bits.push_back(static_cast<unsigned char>(c - '0'));
			}

			if (bits.size() % 8 != 0)
				throw sp::Invalid_Conversion("binary::from_bits: bit count is not a multiple of 8");

			String out(bits.size() / 8, '\0');
			for (std::size_t i = 0; i < out.size(); ++i)
			{
				unsigned char value = 0;
				for (int bit = 0; bit < 8; ++bit)
					value = static_cast<unsigned char>((value << 1) | bits[8 * i + static_cast<std::size_t>(bit)]);
				out[i] = static_cast<char>(value);
			}

			return out;
		}

		cppString hex_dump(const String& bytes, std::size_t bytes_per_line, bool show_ascii)
		{
			if (bytes_per_line == 0)
				throw sp::Invalid_Argument("binary::hex_dump: bytes_per_line must be positive");

			std::ostringstream oss;

			for (std::size_t offset = 0; offset < bytes.size(); offset += bytes_per_line)
			{
				oss << HEX_LOWER[(offset >> 28) & 0xF] << HEX_LOWER[(offset >> 24) & 0xF]
					<< HEX_LOWER[(offset >> 20) & 0xF] << HEX_LOWER[(offset >> 16) & 0xF]
					<< HEX_LOWER[(offset >> 12) & 0xF] << HEX_LOWER[(offset >> 8) & 0xF]
					<< HEX_LOWER[(offset >> 4) & 0xF] << HEX_LOWER[offset & 0xF] << "  ";

				for (std::size_t i = 0; i < bytes_per_line; ++i)
				{
					if (offset + i < bytes.size())
					{
						const auto b = byte_at(bytes, offset + i);
						oss << nibble_hex(static_cast<unsigned char>(b >> 4), false)
							<< nibble_hex(b, false) << ' ';
					}
					else
					{
						oss << "   ";
					}
				}

				if (show_ascii)
				{
					oss << " |";
					for (std::size_t i = 0; i < bytes_per_line; ++i)
					{
						if (offset + i < bytes.size())
						{
							const auto b = byte_at(bytes, offset + i);
							oss << (is_printable_byte(b) ? static_cast<char>(b) : '.');
						}
						else
						{
							oss << ' ';
						}
					}
					oss << '|';
				}

				oss << '\n';
			}

			return cppString{ oss.str() };
		}

		cppString printable(const String& bytes, char placeholder)
		{
			String out(bytes.size(), '\0');
			for (std::size_t i = 0; i < bytes.size(); ++i)
			{
				const auto b = byte_at(bytes, i);
				out[i] = is_printable_byte(b) ? static_cast<char>(b) : placeholder;
			}
			return cppString{ std::move(out) };
		}

		bool is_printable(const String& bytes) noexcept
		{
			for (std::size_t i = 0; i < bytes.size(); ++i)
				if (!is_printable_byte(byte_at(bytes, i)))
					return false;
			return true;
		}

		// ---- bit-level operations -------------------------------------------

		bool get_bit(const String& bytes, std::size_t bit_index)
		{
			const std::size_t index = bit_index / 8;
			if (index >= bytes.size())
				throw sp::Index_Out_Of_Range("binary::get_bit: bit index out of range");
			return ((byte_at(bytes, index) >> (bit_index % 8)) & 1u) != 0;
		}

		void set_bit(String& bytes, std::size_t bit_index, bool value)
		{
			const std::size_t index = bit_index / 8;
			if (index >= bytes.size())
				throw sp::Index_Out_Of_Range("binary::set_bit: bit index out of range");

			const unsigned char mask = static_cast<unsigned char>(1u << (bit_index % 8));
			auto b = byte_at(bytes, index);
			b = value ? static_cast<unsigned char>(b | mask)
				: static_cast<unsigned char>(b & static_cast<unsigned char>(~mask));
			bytes[index] = static_cast<char>(b);
		}

		std::size_t popcount(const String& bytes) noexcept
		{
			std::size_t total = 0;
			for (std::size_t i = 0; i < bytes.size(); ++i)
				total += static_cast<std::size_t>(std::popcount(byte_at(bytes, i)));
			return total;
		}

		std::uint8_t reverse_bits(std::uint8_t byte) noexcept
		{
			auto b = byte;
			b = static_cast<std::uint8_t>(((b & 0xF0u) >> 4) | ((b & 0x0Fu) << 4));
			b = static_cast<std::uint8_t>(((b & 0xCCu) >> 2) | ((b & 0x33u) << 2));
			b = static_cast<std::uint8_t>(((b & 0xAAu) >> 1) | ((b & 0x55u) << 1));
			return b;
		}

		String bit_not(const String& bytes)
		{
			String out(bytes.size(), '\0');
			for (std::size_t i = 0; i < bytes.size(); ++i)
				out[i] = static_cast<char>(static_cast<unsigned char>(~byte_at(bytes, i)));
			return out;
		}

		String bit_and(const String& lhs, const String& rhs) { return bit_op(lhs, rhs, '&'); }
		String bit_or(const String& lhs, const String& rhs)  { return bit_op(lhs, rhs, '|'); }
		String bit_xor(const String& lhs, const String& rhs) { return bit_op(lhs, rhs, '^'); }

		std::uint16_t byteswap16(std::uint16_t value) noexcept
		{
			return static_cast<std::uint16_t>((value << 8) | (value >> 8));
		}

		std::uint32_t byteswap32(std::uint32_t value) noexcept
		{
			return ((value & 0x000000FFu) << 24) | ((value & 0x0000FF00u) << 8)
				| ((value & 0x00FF0000u) >> 8) | ((value & 0xFF000000u) >> 24);
		}

		std::uint64_t byteswap64(std::uint64_t value) noexcept
		{
			return ((value & 0x00000000000000FFull) << 56)
				| ((value & 0x000000000000FF00ull) << 40)
				| ((value & 0x0000000000FF0000ull) << 24)
				| ((value & 0x00000000FF000000ull) << 8)
				| ((value & 0x000000FF00000000ull) >> 8)
				| ((value & 0x0000FF0000000000ull) >> 24)
				| ((value & 0x00FF000000000000ull) >> 40)
				| ((value & 0xFF00000000000000ull) >> 56);
		}

		String reverse_bytes(const String& bytes)
		{
			String out{ bytes };
			std::reverse(out.begin(), out.end());
			return out;
		}

		// ---- buffer scanning -------------------------------------------------

		std::size_t find(const String& data, const String& pattern, std::size_t from) noexcept
		{
			if (pattern.empty())
				return from <= data.size() ? from : String::npos;
			return data.find(pattern, from);
		}

		std::vector<std::size_t> find_all(const String& data, const String& pattern)
		{
			if (pattern.empty())
				throw sp::Invalid_Argument("binary::find_all: pattern must not be empty");

			std::vector<std::size_t> hits;
			std::size_t cursor = 0;
			std::size_t hit = data.find(pattern, cursor);
			while (hit != String::npos)
			{
				hits.push_back(hit);
				cursor = hit + pattern.size();
				hit = data.find(pattern, cursor);
			}
			return hits;
		}

		std::vector<String> split(const String& data, const String& delimiter)
		{
			if (delimiter.empty())
				throw sp::Invalid_Argument("binary::split: delimiter must not be empty");

			std::vector<String> parts;
			std::size_t cursor = 0;
			while (true)
			{
				const std::size_t hit = data.find(delimiter, cursor);
				if (hit == String::npos)
				{
					parts.push_back(data.substr(cursor));
					break;
				}
				parts.push_back(data.substr(cursor, hit - cursor));
				cursor = hit + delimiter.size();
			}
			return parts;
		}

		String slice(const String& data, std::size_t offset, std::size_t length)
		{
			if (offset > data.size())
				throw sp::Index_Out_Of_Range("binary::slice: offset out of range");

			const std::size_t available = data.size() - offset;
			const std::size_t count = length == String::npos ? available : length;
			if (count > available)
				throw sp::Index_Out_Of_Range("binary::slice: length out of range");
			return data.substr(offset, count);
		}

		// ---- raw file I/O ----------------------------------------------------

		String load_file(const cppString& path)
		{
			std::ifstream input{ to_path(path), std::ios::binary };
			if (!input)
				throw sp::Invalid_Operation("binary::load_file: cannot open file for reading");

			String bytes{ std::istreambuf_iterator<char>{ input }, std::istreambuf_iterator<char>{} };
			if (input.bad())
				throw sp::Invalid_Operation("binary::load_file: read failed");
			return bytes;
		}

		void save_file(const cppString& path, const String& bytes)
		{
			std::ofstream output{ to_path(path), std::ios::binary | std::ios::trunc };
			if (!output)
				throw sp::Invalid_Operation("binary::save_file: cannot open file for writing");

			if (!bytes.empty())
				output.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
			output.flush();

			if (!output)
				throw sp::Invalid_Operation("binary::save_file: write failed");
		}

		// ---- Reader ----------------------------------------------------------

		Reader::Reader(String bytes, Endian order)
			: m_data{ std::move(bytes) }, m_endian{ order }
		{
		}

		Reader Reader::from_file(const cppString& path, Endian order)
		{
			return Reader{ load_file(path), order };
		}

		Reader Reader::from_hex(const cppString& text, Endian order)
		{
			return Reader{ sp::binary::from_hex(text), order };
		}

		const String& Reader::data() const noexcept { return m_data; }
		std::size_t Reader::size() const noexcept { return m_data.size(); }
		std::size_t Reader::tell() const noexcept { return m_pos; }
		std::size_t Reader::remaining() const noexcept { return m_data.size() - m_pos; }
		bool Reader::eof() const noexcept { return m_pos >= m_data.size(); }
		Endian Reader::endian() const noexcept { return m_endian; }
		void Reader::set_endian(Endian value) noexcept { m_endian = value; }
		void Reader::rewind() noexcept { m_pos = 0; }

		void Reader::require(std::size_t count) const
		{
			if (count > remaining())
				throw sp::Index_Out_Of_Range("binary::Reader: read past end of buffer");
		}

		Endian Reader::effective(std::optional<Endian> order) const noexcept
		{
			return order.value_or(m_endian);
		}

		void Reader::seek(std::size_t position)
		{
			if (position > m_data.size())
				throw sp::Index_Out_Of_Range("binary::Reader::seek: position out of range");
			m_pos = position;
		}

		void Reader::skip(std::size_t count)
		{
			require(count);
			m_pos += count;
		}

		void Reader::align(std::size_t boundary)
		{
			if (boundary == 0)
				throw sp::Invalid_Argument("binary::Reader::align: boundary must be positive");
			const std::size_t padding = (boundary - (m_pos % boundary)) % boundary;
			require(padding);
			m_pos += padding;
		}

		std::uint64_t Reader::read_uint(std::size_t width, Endian order)
		{
			require(width);
			std::uint64_t value = 0;

			if (order == Endian::Little)
			{
				for (std::size_t i = 0; i < width; ++i)
					value |= static_cast<std::uint64_t>(byte_at(m_data, m_pos + i)) << (8 * i);
			}
			else
			{
				for (std::size_t i = 0; i < width; ++i)
					value = (value << 8) | byte_at(m_data, m_pos + i);
			}

			m_pos += width;
			return value;
		}

		std::uint8_t Reader::u8()
		{
			require(1);
			return byte_at(m_data, m_pos++);
		}

		std::int8_t Reader::i8()
		{
			return std::bit_cast<std::int8_t>(u8());
		}

		std::uint16_t Reader::u16(std::optional<Endian> order)
		{
			return static_cast<std::uint16_t>(read_uint(2, effective(order)));
		}

		std::int16_t Reader::i16(std::optional<Endian> order)
		{
			return std::bit_cast<std::int16_t>(u16(order));
		}

		std::uint32_t Reader::u32(std::optional<Endian> order)
		{
			return static_cast<std::uint32_t>(read_uint(4, effective(order)));
		}

		std::int32_t Reader::i32(std::optional<Endian> order)
		{
			return std::bit_cast<std::int32_t>(u32(order));
		}

		std::uint64_t Reader::u64(std::optional<Endian> order)
		{
			return read_uint(8, effective(order));
		}

		std::int64_t Reader::i64(std::optional<Endian> order)
		{
			return std::bit_cast<std::int64_t>(u64(order));
		}

		float Reader::f32(std::optional<Endian> order)
		{
			return std::bit_cast<float>(u32(order));
		}

		double Reader::f64(std::optional<Endian> order)
		{
			return std::bit_cast<double>(u64(order));
		}

		String Reader::bytes(std::size_t count)
		{
			require(count);
			String out = m_data.substr(m_pos, count);
			m_pos += count;
			return out;
		}

		String Reader::peek(std::size_t count) const
		{
			require(count);
			return m_data.substr(m_pos, count);
		}

		String Reader::rest()
		{
			return bytes(remaining());
		}

		String Reader::read_until(std::uint8_t delimiter, bool consume_delimiter)
		{
			const std::size_t hit = m_data.find(static_cast<char>(delimiter), m_pos);
			if (hit == String::npos)
				return rest();

			String out = m_data.substr(m_pos, hit - m_pos);
			m_pos = consume_delimiter ? hit + 1 : hit;
			return out;
		}

		String Reader::cstring()
		{
			const std::size_t hit = m_data.find('\0', m_pos);
			if (hit == String::npos)
				throw sp::Invalid_Conversion("binary::Reader::cstring: missing NUL terminator");

			String out = m_data.substr(m_pos, hit - m_pos);
			m_pos = hit + 1;
			return out;
		}

		String Reader::line()
		{
			String out = read_until(static_cast<std::uint8_t>('\n'), true);
			if (!out.empty() && out.back() == '\r')
				out.pop_back();
			return out;
		}

		std::uint64_t Reader::varint()
		{
			std::uint64_t result = 0;
			int shift = 0;

			for (int i = 0; i < 10; ++i)
			{
				const std::uint8_t byte = u8();
				const std::uint64_t payload = static_cast<std::uint64_t>(byte & 0x7Fu);

				if (shift >= 64)
				{
					if (payload != 0)
						throw sp::Invalid_Conversion("binary::Reader::varint: value overflows 64 bits");
				}
				else
				{
					if (shift == 63 && payload > 1)
						throw sp::Invalid_Conversion("binary::Reader::varint: value overflows 64 bits");
					result |= payload << shift;
				}

				if ((byte & 0x80u) == 0)
					return result;
				shift += 7;
			}

			throw sp::Invalid_Conversion("binary::Reader::varint: malformed or overlong encoding");
		}

		std::int64_t Reader::svarint()
		{
			const std::uint64_t encoded = varint();
			const std::uint64_t decoded = (encoded >> 1) ^ (0ull - (encoded & 1ull));
			return std::bit_cast<std::int64_t>(decoded);
		}

		bool Reader::match(const String& signature, bool consume)
		{
			if (signature.size() > remaining())
				return false;
			if (m_data.compare(m_pos, signature.size(), signature) != 0)
				return false;
			if (consume)
				m_pos += signature.size();
			return true;
		}

		bool Reader::skip_to(const String& signature, bool consume)
		{
			if (signature.empty())
				throw sp::Invalid_Argument("binary::Reader::skip_to: signature must not be empty");

			const std::size_t hit = m_data.find(signature, m_pos);
			if (hit == String::npos)
				return false;
			m_pos = consume ? hit + signature.size() : hit;
			return true;
		}

		std::size_t Reader::find(const String& signature, std::size_t from) const noexcept
		{
			if (signature.empty())
				return from <= m_data.size() ? from : String::npos;
			return m_data.find(signature, from);
		}

		// ---- Writer ----------------------------------------------------------

		Writer::Writer(Endian order) : m_endian{ order } {}

		void Writer::reserve(std::size_t capacity) { m_data.reserve(capacity); }
		void Writer::clear() noexcept { m_data.clear(); }
		std::size_t Writer::size() const noexcept { return m_data.size(); }
		const String& Writer::data() const noexcept { return m_data; }
		Endian Writer::endian() const noexcept { return m_endian; }
		void Writer::set_endian(Endian value) noexcept { m_endian = value; }

		String Writer::take()
		{
			String out = std::move(m_data);
			m_data.clear();
			return out;
		}

		cppString Writer::to_hex() const
		{
			return sp::binary::to_hex(m_data);
		}

		Endian Writer::effective(std::optional<Endian> order) const noexcept
		{
			return order.value_or(m_endian);
		}

		void Writer::write_uint(std::uint64_t value, std::size_t width, Endian order)
		{
			if (order == Endian::Little)
			{
				for (std::size_t i = 0; i < width; ++i)
					m_data.push_back(static_cast<char>(static_cast<unsigned char>((value >> (8 * i)) & 0xFFu)));
			}
			else
			{
				for (std::size_t i = 0; i < width; ++i)
					m_data.push_back(static_cast<char>(static_cast<unsigned char>((value >> (8 * (width - 1 - i))) & 0xFFu)));
			}
		}

		void Writer::align(std::size_t boundary, std::uint8_t fill)
		{
			if (boundary == 0)
				throw sp::Invalid_Argument("binary::Writer::align: boundary must be positive");
			const std::size_t padding = (boundary - (m_data.size() % boundary)) % boundary;
			pad(padding, fill);
		}

		Writer& Writer::u8(std::uint8_t value)
		{
			m_data.push_back(static_cast<char>(value));
			return *this;
		}

		Writer& Writer::i8(std::int8_t value)
		{
			return u8(std::bit_cast<std::uint8_t>(value));
		}

		Writer& Writer::u16(std::uint16_t value, std::optional<Endian> order)
		{
			write_uint(value, 2, effective(order));
			return *this;
		}

		Writer& Writer::i16(std::int16_t value, std::optional<Endian> order)
		{
			return u16(std::bit_cast<std::uint16_t>(value), order);
		}

		Writer& Writer::u32(std::uint32_t value, std::optional<Endian> order)
		{
			write_uint(value, 4, effective(order));
			return *this;
		}

		Writer& Writer::i32(std::int32_t value, std::optional<Endian> order)
		{
			return u32(std::bit_cast<std::uint32_t>(value), order);
		}

		Writer& Writer::u64(std::uint64_t value, std::optional<Endian> order)
		{
			write_uint(value, 8, effective(order));
			return *this;
		}

		Writer& Writer::i64(std::int64_t value, std::optional<Endian> order)
		{
			return u64(std::bit_cast<std::uint64_t>(value), order);
		}

		Writer& Writer::f32(float value, std::optional<Endian> order)
		{
			return u32(std::bit_cast<std::uint32_t>(value), order);
		}

		Writer& Writer::f64(double value, std::optional<Endian> order)
		{
			return u64(std::bit_cast<std::uint64_t>(value), order);
		}

		Writer& Writer::bytes(const String& value)
		{
			m_data += value;
			return *this;
		}

		Writer& Writer::raw(const void* value, std::size_t count)
		{
			if (count != 0 && value == nullptr)
				throw sp::Invalid_Argument("binary::Writer::raw: null buffer with non-zero size");
			if (count != 0)
				m_data.append(static_cast<const char*>(value), count);
			return *this;
		}

		Writer& Writer::cstring(const String& value)
		{
			m_data += value;
			m_data.push_back('\0');
			return *this;
		}

		Writer& Writer::varint(std::uint64_t value)
		{
			while (value >= 0x80u)
			{
				m_data.push_back(static_cast<char>(static_cast<unsigned char>((value & 0x7Fu) | 0x80u)));
				value >>= 7;
			}
			m_data.push_back(static_cast<char>(static_cast<unsigned char>(value)));
			return *this;
		}

		Writer& Writer::svarint(std::int64_t value)
		{
			const std::uint64_t raw = std::bit_cast<std::uint64_t>(value);
			const std::uint64_t zigzag = (raw << 1) ^ (value < 0 ? ~0ull : 0ull);
			return varint(zigzag);
		}

		Writer& Writer::pad(std::size_t count, std::uint8_t fill)
		{
			m_data.append(count, static_cast<char>(fill));
			return *this;
		}
	}
}
