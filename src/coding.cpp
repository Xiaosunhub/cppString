// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

#include "coding.h"
#include <cctype>
#include <cstdint>
#include <sstream>

namespace
{
	const char* const BASE64_ALPHABET =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int base64_value(unsigned char c)
	{
		if (c >= 'A' && c <= 'Z') return c - 'A';
		if (c >= 'a' && c <= 'z') return c - 'a' + 26;
		if (c >= '0' && c <= '9') return c - '0' + 52;
		if (c == '+') return 62;
		if (c == '/') return 63;
		return -1;
	}

	int hex_value(unsigned char c)
	{
		if (c >= '0' && c <= '9') return c - '0';
		if (c >= 'a' && c <= 'f') return c - 'a' + 10;
		if (c >= 'A' && c <= 'F') return c - 'A' + 10;
		return -1;
	}

	bool is_unreserved(unsigned char c)
	{
		return (c >= 'A' && c <= 'Z')
			|| (c >= 'a' && c <= 'z')
			|| (c >= '0' && c <= '9')
			|| c == '-' || c == '_' || c == '.' || c == '~';
	}

	char hex_digit(unsigned int value, bool uppercase)
	{
		static const char* lower = "0123456789abcdef";
		static const char* upper = "0123456789ABCDEF";
		return uppercase ? upper[value & 0x0F] : lower[value & 0x0F];
	}
}

sp::cppString sp::coding::toBase64(const cppString& data)
{
	const String& src = data.s_str();

	cppString result;
	result.reserve(((src.size() + 2) / 3) * 4);

	std::size_t i = 0;
	while (i + 2 < src.size())
	{
		const unsigned int block =
			(static_cast<unsigned char>(src[i]) << 16) |
			(static_cast<unsigned char>(src[i + 1]) << 8) |
			(static_cast<unsigned char>(src[i + 2]));

		result += BASE64_ALPHABET[(block >> 18) & 0x3F];
		result += BASE64_ALPHABET[(block >> 12) & 0x3F];
		result += BASE64_ALPHABET[(block >> 6) & 0x3F];
		result += BASE64_ALPHABET[block & 0x3F];

		i += 3;
	}

	const std::size_t rest = src.size() - i;

	if (rest == 1)
	{
		const unsigned int block = static_cast<unsigned char>(src[i]) << 16;

		result += BASE64_ALPHABET[(block >> 18) & 0x3F];
		result += BASE64_ALPHABET[(block >> 12) & 0x3F];
		result += '=';
		result += '=';
	}
	else if (rest == 2)
	{
		const unsigned int block =
			(static_cast<unsigned char>(src[i]) << 16) |
			(static_cast<unsigned char>(src[i + 1]) << 8);

		result += BASE64_ALPHABET[(block >> 18) & 0x3F];
		result += BASE64_ALPHABET[(block >> 12) & 0x3F];
		result += BASE64_ALPHABET[(block >> 6) & 0x3F];
		result += '=';
	}

	return result;
}

sp::cppString sp::coding::fromBase64(const cppString& text)
{
	const String& src = text.s_str();

	String bytes;
	bytes.reserve((src.size() / 4) * 3);

	unsigned int accumulator = 0;
	int bits = 0;

	for (char raw : src)
	{
		const unsigned char c = static_cast<unsigned char>(raw);

		if (std::isspace(c))
			continue;

		if (c == '=')
			break; // padding: stop decoding

		const int value = base64_value(c);
		if (value < 0)
			throw sp::Invalid_Conversion("Invalid base64 character in fromBase64.");

		accumulator = (accumulator << 6) | static_cast<unsigned int>(value);
		bits += 6;

		if (bits >= 8)
		{
			bits -= 8;
			bytes += static_cast<char>((accumulator >> bits) & 0xFF);
		}
	}

	return cppString(bytes);
}

sp::cppString sp::coding::toHex(const cppString& data, bool uppercase)
{
	const String& src = data.s_str();

	cppString result;
	result.reserve(src.size() * 2);

	for (char raw : src)
	{
		const unsigned char c = static_cast<unsigned char>(raw);
		result += hex_digit(c >> 4, uppercase);
		result += hex_digit(c, uppercase);
	}

	return result;
}

sp::cppString sp::coding::fromHex(const cppString& text)
{
	const String& src = text.s_str();

	String bytes;
	bytes.reserve(src.size() / 2);

	int high = -1;

	for (char raw : src)
	{
		const unsigned char c = static_cast<unsigned char>(raw);

		if (std::isspace(c))
			continue;

		const int value = hex_value(c);
		if (value < 0)
			throw sp::Invalid_Conversion("Invalid hex character in fromHex.");

		if (high < 0)
		{
			high = value;
		}
		else
		{
			bytes += static_cast<char>((high << 4) | value);
			high = -1;
		}
	}

	if (high >= 0)
		throw sp::Invalid_Conversion("Odd number of hex digits in fromHex.");

	return cppString(bytes);
}

sp::cppString sp::coding::urlEncode(const cppString& data)
{
	const String& src = data.s_str();

	cppString result;
	result.reserve(src.size());

	for (char raw : src)
	{
		const unsigned char c = static_cast<unsigned char>(raw);

		if (is_unreserved(c))
		{
			result += static_cast<char>(c);
		}
		else
		{
			result += '%';
			result += hex_digit(c >> 4, true);
			result += hex_digit(c, true);
		}
	}

	return result;
}

sp::cppString sp::coding::urlDecode(const cppString& text)
{
	const String& src = text.s_str();

	String bytes;
	bytes.reserve(src.size());

	for (std::size_t i = 0; i < src.size(); ++i)
	{
		const unsigned char c = static_cast<unsigned char>(src[i]);

		if (c == '+')
		{
			bytes += ' ';
		}
		else if (c == '%')
		{
			if (i + 2 >= src.size())
				throw sp::Invalid_Conversion("Truncated percent sequence in urlDecode.");

			const int high = hex_value(static_cast<unsigned char>(src[i + 1]));
			const int low = hex_value(static_cast<unsigned char>(src[i + 2]));

			if (high < 0 || low < 0)
				throw sp::Invalid_Conversion("Invalid percent sequence in urlDecode.");

			bytes += static_cast<char>((high << 4) | low);
			i += 2;
		}
		else
		{
			bytes += static_cast<char>(c);
		}
	}

	return cppString(bytes);
}

namespace
{
	bool all_digits_in(const String& src, const char* digits)
	{
		return !src.empty() && src.find_first_not_of(digits) == String::npos;
	}
}

bool sp::coding::is_binary(const cppString& text) noexcept
{
	return all_digits_in(text.s_str(), "01");
}

sp::cppString sp::coding::toBinary(const cppString& data)
{
	String bits;
	bits.reserve(data.size() * 8);

	for (const char c : data.s_str())
	{
		const unsigned char byte = static_cast<unsigned char>(c);

		for (int shift = 7; shift >= 0; --shift)
			bits += static_cast<char>('0' + ((byte >> shift) & 1));
	}

	return cppString(std::move(bits));
}

sp::cppString sp::coding::fromBinary(const cppString& text)
{
	const String& src = text.s_str();

	if (src.empty() || src.size() % 8 != 0)
		throw sp::Invalid_Conversion("Binary text length must be a non-zero multiple of 8 in fromBinary.");

	if (!all_digits_in(src, "01"))
		throw sp::Invalid_Conversion("Binary text may only contain '0' and '1' in fromBinary.");

	String bytes;
	bytes.reserve(src.size() / 8);

	for (std::size_t i = 0; i < src.size(); i += 8)
	{
		unsigned char byte = 0;

		for (std::size_t j = 0; j < 8; ++j)
			byte = static_cast<unsigned char>((byte << 1) | (src[i + j] - '0'));

		bytes += static_cast<char>(byte);
	}

	return cppString(std::move(bytes));
}

bool sp::coding::is_octal(const cppString& text) noexcept
{
	return all_digits_in(text.s_str(), "01234567");
}

sp::cppString sp::coding::toOctal(const cppString& data)
{
	String digits;
	digits.reserve(data.size() * 3);

	for (const char c : data.s_str())
	{
		const unsigned char byte = static_cast<unsigned char>(c);

		digits += static_cast<char>('0' + ((byte >> 6) & 7));
		digits += static_cast<char>('0' + ((byte >> 3) & 7));
		digits += static_cast<char>('0' + (byte & 7));
	}

	return cppString(std::move(digits));
}

sp::cppString sp::coding::fromOctal(const cppString& text)
{
	const String& src = text.s_str();

	if (src.empty() || src.size() % 3 != 0)
		throw sp::Invalid_Conversion("Octal text length must be a non-zero multiple of 3 in fromOctal.");

	if (!all_digits_in(src, "01234567"))
		throw sp::Invalid_Conversion("Octal text may only contain digits 0-7 in fromOctal.");

	String bytes;
	bytes.reserve(src.size() / 3);

	for (std::size_t i = 0; i < src.size(); i += 3)
	{
		const unsigned byte = static_cast<unsigned>(
			(src[i] - '0') * 64 + (src[i + 1] - '0') * 8 + (src[i + 2] - '0'));

		bytes += static_cast<char>(byte);
	}

	return cppString(std::move(bytes));
}

sp::cppString sp::coding::toBase(unsigned long long value, unsigned base)
{
	if (base < 2 || base > 36)
		throw sp::Invalid_Argument("Base must be between 2 and 36 in toBase.");

	if (value == 0)
		return cppString{ "0" };

	static constexpr char DIGITS[] = "0123456789abcdefghijklmnopqrstuvwxyz";

	String result;

	while (value > 0)
	{
		result += DIGITS[value % base];
		value /= base;
	}

	std::ranges::reverse(result);

	return cppString(std::move(result));
}

long long sp::coding::fromBase(const cppString& text, unsigned base)
{
	if (base < 2 || base > 36)
		throw sp::Invalid_Argument("Base must be between 2 and 36 in fromBase.");

	String src = text.s_str();

	if (src.empty())
		throw sp::Invalid_Conversion("Cannot convert empty string in fromBase.");

	bool negative = false;
	std::size_t i = 0;

	if (src[0] == '-' || src[0] == '+')
	{
		negative = (src[0] == '-');
		i = 1;

		if (src.size() == 1)
			throw sp::Invalid_Conversion("Sign-only string in fromBase.");
	}

	unsigned long long value = 0;

	for (; i < src.size(); ++i)
	{
		unsigned char c = static_cast<unsigned char>(src[i]);
		unsigned digit;

		if (c >= '0' && c <= '9')      digit = static_cast<unsigned>(c - '0');
		else if (c >= 'a' && c <= 'z') digit = static_cast<unsigned>(c - 'a' + 10);
		else if (c >= 'A' && c <= 'Z') digit = static_cast<unsigned>(c - 'A' + 10);
		else
			throw sp::Invalid_Conversion("Invalid character in fromBase.");

		if (digit >= base)
			throw sp::Invalid_Conversion("Digit out of range for the given base in fromBase.");

		const unsigned long long next = value * base + digit;

		if (next / base != value || next < value)
			throw sp::Invalid_Conversion("Overflow in fromBase.");

		value = next;
	}

	constexpr unsigned long long ULL_MAX_AS_LL =
		static_cast<unsigned long long>(std::numeric_limits<long long>::max());

	if (value > (negative ? ULL_MAX_AS_LL + 1ULL : ULL_MAX_AS_LL))
		throw sp::Invalid_Conversion("Overflow in fromBase.");

	if (negative)
		return static_cast<long long>(0ULL - value);

	return static_cast<long long>(value);
}

namespace
{
	constexpr char BASE32_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

	int base32_value(unsigned char c)
	{
		if (c >= 'A' && c <= 'Z') return c - 'A';
		if (c >= 'a' && c <= 'z') return c - 'a';
		if (c >= '2' && c <= '7') return c - '2' + 26;
		return -1;
	}
}

sp::cppString sp::coding::toBase32(const cppString& data)
{
	const String& src = data.s_str();

	String result;
	result.reserve((src.size() + 4) / 5 * 8);

	std::uint32_t acc = 0;
	int bits = 0;

	for (const char ch : src)
	{
		acc = (acc << 8) | static_cast<unsigned char>(ch);
		bits += 8;

		while (bits >= 5)
		{
			bits -= 5;
			result += BASE32_ALPHABET[(acc >> bits) & 0x1Fu];
		}
	}

	if (bits > 0)
		result += BASE32_ALPHABET[(acc << (5 - bits)) & 0x1Fu];

	while (result.size() % 8 != 0)
		result += '=';

	return cppString(std::move(result));
}

sp::cppString sp::coding::fromBase32(const cppString& text)
{
	const String& src = text.s_str();

	String result;
	result.reserve(src.size() * 5 / 8);

	std::uint32_t acc = 0;
	int bits = 0;
	bool padded = false;

	for (const char ch : src)
	{
		if (ch == '=')
		{
			padded = true;
			continue;
		}

		if (padded)
			throw sp::Invalid_Conversion("Data after '=' padding in fromBase32.");

		const int value = base32_value(static_cast<unsigned char>(ch));

		if (value < 0)
			throw sp::Invalid_Conversion("Invalid base32 character in fromBase32.");

		acc = (acc << 5) | static_cast<std::uint32_t>(value);
		bits += 5;

		if (bits >= 8)
		{
			bits -= 8;
			result += static_cast<char>((acc >> bits) & 0xFFu);
		}
	}

	return cppString(std::move(result));
}

namespace
{
	constexpr char BASE64URL_ALPHABET[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

	int base64url_value(unsigned char c)
	{
		if (c >= 'A' && c <= 'Z') return c - 'A';
		if (c >= 'a' && c <= 'z') return c - 'a' + 26;
		if (c >= '0' && c <= '9') return c - '0' + 52;
		if (c == '-')              return 62;
		if (c == '_')              return 63;
		return -1;
	}
}

sp::cppString sp::coding::toBase64Url(const cppString& data)
{
	const String& src = data.s_str();

	cppString result;
	result.reserve((src.size() * 8 + 5) / 6);

	std::size_t i = 0;
	while (i + 2 < src.size())
	{
		const unsigned block =
			(static_cast<unsigned char>(src[i]) << 16) |
			(static_cast<unsigned char>(src[i + 1]) << 8) |
			(static_cast<unsigned char>(src[i + 2]));

		result += BASE64URL_ALPHABET[(block >> 18) & 0x3F];
		result += BASE64URL_ALPHABET[(block >> 12) & 0x3F];
		result += BASE64URL_ALPHABET[(block >> 6) & 0x3F];
		result += BASE64URL_ALPHABET[block & 0x3F];

		i += 3;
	}

	const std::size_t rest = src.size() - i;

	if (rest == 1)
	{
		const unsigned block = static_cast<unsigned char>(src[i]) << 16;

		result += BASE64URL_ALPHABET[(block >> 18) & 0x3F];
		result += BASE64URL_ALPHABET[(block >> 12) & 0x3F];
	}
	else if (rest == 2)
	{
		const unsigned block =
			(static_cast<unsigned char>(src[i]) << 16) |
			(static_cast<unsigned char>(src[i + 1]) << 8);

		result += BASE64URL_ALPHABET[(block >> 18) & 0x3F];
		result += BASE64URL_ALPHABET[(block >> 12) & 0x3F];
		result += BASE64URL_ALPHABET[(block >> 6) & 0x3F];
	}

	return result;
}

sp::cppString sp::coding::fromBase64Url(const cppString& text)
{
	String src = text.s_str();

	// Tolerate optional padding, then validate the length class.
	while (!src.empty() && src.back() == '=')
		src.pop_back();

	if (src.size() % 4 == 1)
		throw sp::Invalid_Conversion("Invalid base64url length in fromBase64Url.");

	cppString result;
	result.reserve((src.size() * 6) / 8);

	unsigned accumulator = 0;
	int bits = 0;

	for (const char ch : src)
	{
		const int value = base64url_value(static_cast<unsigned char>(ch));

		if (value < 0)
			throw sp::Invalid_Conversion("Invalid character in base64url text in fromBase64Url.");

		accumulator = (accumulator << 6) | static_cast<unsigned>(value);
		bits += 6;

		if (bits >= 8)
		{
			bits -= 8;
			result += static_cast<char>((accumulator >> bits) & 0xFF);
		}
	}

	return result;
}

// ---- classical ciphers ----

namespace
{
	char caesar_shift(char c, int shift)
	{
		const int s = ((shift % 26) + 26) % 26;

		if (c >= 'a' && c <= 'z')
			return static_cast<char>('a' + (c - 'a' + s) % 26);

		if (c >= 'A' && c <= 'Z')
			return static_cast<char>('A' + (c - 'A' + s) % 26);

		return c;
	}
}

sp::cppString sp::coding::rot13(const cppString& text)
{
	return caesar(text, 13);
}

sp::cppString sp::coding::caesar(const cppString& text, int shift)
{
	String result = text.s_str();

	for (auto& c : result)
		c = caesar_shift(c, shift);

	return cppString(std::move(result));
}

// ---- Morse code ----

namespace
{
	const char* morse_of(char c)
	{
		switch (static_cast<char>(std::toupper(static_cast<unsigned char>(c))))
		{
		case 'A': return ".-";    case 'B': return "-...";  case 'C': return "-.-.";
		case 'D': return "-..";   case 'E': return ".";     case 'F': return "..-.";
		case 'G': return "--.";   case 'H': return "....";  case 'I': return "..";
		case 'J': return ".---";  case 'K': return "-.-";   case 'L': return ".-..";
		case 'M': return "--";    case 'N': return "-.";    case 'O': return "---";
		case 'P': return ".--.";  case 'Q': return "--.-";  case 'R': return ".-.";
		case 'S': return "...";   case 'T': return "-";     case 'U': return "..-";
		case 'V': return "...-";  case 'W': return ".--";   case 'X': return "-..-";
		case 'Y': return "-.--";  case 'Z': return "--..";
		case '0': return "-----"; case '1': return ".----"; case '2': return "..---";
		case '3': return "...--"; case '4': return "....-"; case '5': return ".....";
		case '6': return "-...."; case '7': return "--..."; case '8': return "---..";
		case '9': return "----.";
		case '.': return ".-.-.-"; case ',': return "--..--"; case '?': return "..--..";
		case '!': return "-.-.--"; case '\'': return ".----."; case '"': return ".-..-.";
		default:  return nullptr;
		}
	}

	char morse_to_char(const String& group)
	{
		for (char c = 'A'; c <= 'Z'; ++c)
		{
			if (group == morse_of(c))
				return c;
		}

		for (char c = '0'; c <= '9'; ++c)
		{
			if (group == morse_of(c))
				return c;
		}

		for (const char c : { '.', ',', '?', '!', '\'', '\"' })
		{
			if (group == morse_of(c))
				return c;
		}

		return '\0';
	}
}

sp::cppString sp::coding::toMorse(const cppString& text)
{
	String result;
	bool first_letter_of_word = true;

	for (const char c : text.s_str())
	{
		if (c == ' ' || c == '\t' || c == '\n')
		{
			if (!first_letter_of_word) // suppress leading/duplicate separators
			{
				result += " / ";
				first_letter_of_word = true;
			}
			continue;
		}

		const char* code = morse_of(c);

		if (code == nullptr)
			throw sp::Invalid_Conversion("Character has no Morse equivalent in toMorse.");

		if (!first_letter_of_word)
			result += ' ';

		result += code;
		first_letter_of_word = false;
	}

	// Trim a trailing word separator.
	if (result.size() >= 3 && result.compare(result.size() - 3, 3, " / ") == 0)
		result.resize(result.size() - 3);

	return cppString(std::move(result));
}

sp::cppString sp::coding::fromMorse(const cppString& text)
{
	String result;

	std::istringstream stream{ text.s_str() };
	String group;

	while (stream >> group)
	{
		if (group == "/")
		{
			if (!result.empty() && result.back() != ' ')
				result += ' ';

			continue;
		}

		const char decoded = morse_to_char(group);

		if (decoded == '\0')
			throw sp::Invalid_Conversion("Unknown Morse group in fromMorse.");

		result += decoded;
	}

	return cppString(std::move(result));
}

namespace
{
	constexpr char BASE58_ALPHABET[] =
		"123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

	int base58_value(unsigned char c)
	{
		if (c >= '1' && c <= '9') return c - '1';
		if (c >= 'A' && c <= 'H') return c - 'A' + 9;
		if (c >= 'J' && c <= 'N') return c - 'J' + 17;
		if (c >= 'P' && c <= 'Z') return c - 'P' + 22;
		if (c >= 'a' && c <= 'k') return c - 'a' + 33;
		if (c >= 'm' && c <= 'z') return c - 'm' + 44;
		return -1;
	}
}

sp::cppString sp::coding::toBase58(const cppString& data)
{
	String work = data.s_str(); // mutable copy: divided in place below

	// Count leading zero bytes; each becomes one '1'.
	std::size_t zeros = 0;

	while (zeros < work.size() && work[zeros] == '\0')
		++zeros;

	// Big-endian base-256 -> base-58 via repeated division.
	String digits; // little-endian base-58 digits
	std::size_t start = zeros;

	while (start < work.size())
	{
		unsigned int carry = 0;

		for (std::size_t i = start; i < work.size(); ++i)
		{
			const unsigned int value = carry * 256u
				+ static_cast<unsigned char>(work[i]);
			work[i] = static_cast<char>(value / 58u);
			carry = value % 58u;
		}

		digits += BASE58_ALPHABET[carry];

		while (start < work.size() && work[start] == '\0')
			++start;
	}

	String result(zeros, '1');

	for (std::size_t i = digits.size(); i-- > 0;)
		result += digits[i];

	return cppString(std::move(result));
}

sp::cppString sp::coding::fromBase58(const cppString& text)
{
	const String& src = text.s_str();

	std::size_t ones = 0;

	while (ones < src.size() && src[ones] == '1')
		++ones;

	// Map to base-58 digit values (most significant first) up front;
	// the division loop below keeps working in digit space.
	String digits;
	digits.reserve(src.size() - ones);

	for (std::size_t i = ones; i < src.size(); ++i)
	{
		const int d = base58_value(static_cast<unsigned char>(src[i]));

		if (d < 0)
			throw sp::Invalid_Conversion("Invalid base58 character in fromBase58.");

		digits += static_cast<char>(d);
	}

	String bytes; // little-endian base-256
	std::size_t start = 0;

	while (start < digits.size())
	{
		unsigned int carry = 0;

		for (std::size_t i = start; i < digits.size(); ++i)
		{
			const unsigned int value = carry * 58u
				+ static_cast<unsigned char>(digits[i]);
			digits[i] = static_cast<char>(value / 256u);
			carry = value % 256u;
		}

		bytes += static_cast<char>(carry);

		while (start < digits.size() && digits[start] == '\0')
			++start;
	}

	String result(ones, '\0');

	for (std::size_t i = bytes.size(); i-- > 0;)
		result += bytes[i];

	return cppString(std::move(result));
}
