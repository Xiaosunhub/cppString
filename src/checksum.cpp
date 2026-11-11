// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

#include "checksum.h"

namespace
{
	struct Crc32Table
	{
		std::uint32_t entries[256]{};

		constexpr Crc32Table() noexcept
		{
			for (std::uint32_t i = 0; i < 256; ++i)
			{
				std::uint32_t c = i;

				for (int k = 0; k < 8; ++k)
					c = (c & 1u) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);

				entries[i] = c;
			}
		}
	};

	constexpr Crc32Table CRC32_TABLE{};

	String hex_string(std::uint64_t value, int digits)
	{
		String out(static_cast<std::size_t>(digits), '0');

		for (int i = digits - 1; i >= 0; --i)
		{
			out[static_cast<std::size_t>(i)] = "0123456789abcdef"[value & 0xF];
			value >>= 4;
		}

		return out;
	}
}

std::uint32_t sp::checksum::crc32(const cppString& data) noexcept
{
	std::uint32_t crc = 0xFFFFFFFFu;

	for (const char ch : data.s_str())
	{
		const unsigned char byte = static_cast<unsigned char>(ch);
		crc = CRC32_TABLE.entries[(crc ^ byte) & 0xFFu] ^ (crc >> 8);
	}

	return crc ^ 0xFFFFFFFFu;
}

sp::cppString sp::checksum::crc32_hex(const cppString& data)
{
	return cppString{ hex_string(crc32(data), 8) };
}

std::uint64_t sp::checksum::fnv1a64(const cppString& data) noexcept
{
	std::uint64_t hash = 0xCBF29CE484222325ull;

	for (const char ch : data.s_str())
	{
		hash ^= static_cast<unsigned char>(ch);
		hash *= 0x100000001B3ull;
	}

	return hash;
}

sp::cppString sp::checksum::fnv1a64_hex(const cppString& data)
{
	return cppString{ hex_string(fnv1a64(data), 16) };
}

std::uint32_t sp::checksum::adler32(const cppString& data) noexcept
{
	constexpr std::uint32_t MOD_ADLER = 65521u;

	std::uint32_t a = 1;
	std::uint32_t b = 0;

	for (const char ch : data.s_str())
	{
		a = (a + static_cast<unsigned char>(ch)) % MOD_ADLER;
		b = (b + a) % MOD_ADLER;
	}

	return (b << 16) | a;
}

sp::cppString sp::checksum::adler32_hex(const cppString& data)
{
	return cppString{ hex_string(adler32(data), 8) };
}

namespace
{
	// Per-round left-rotate amounts (RFC 1321).
	constexpr int MD5_S[64] =
	{
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
	};

	// K[i] = floor(2^32 * abs(sin(i + 1))).
	constexpr std::uint32_t MD5_K[64] =
	{
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
	};

	constexpr std::uint32_t rotl32(std::uint32_t value, int count) noexcept
	{
		return (value << count) | (value >> (32 - count));
	}
}

sp::cppString sp::checksum::md5(const cppString& data)
{
	const String& src = data.s_str();

	// Padded message: data + 0x80 + zeros + 64-bit little-endian bit length.
	String msg = src;
	msg += static_cast<char>(0x80);

	while (msg.size() % 64 != 56)
		msg += '\0';

	const std::uint64_t bit_len = static_cast<std::uint64_t>(src.size()) * 8u;

	for (int i = 0; i < 8; ++i)
		msg += static_cast<char>((bit_len >> (i * 8)) & 0xFFu);

	std::uint32_t a0 = 0x67452301;
	std::uint32_t b0 = 0xefcdab89;
	std::uint32_t c0 = 0x98badcfe;
	std::uint32_t d0 = 0x10325476;

	for (std::size_t block = 0; block < msg.size(); block += 64)
	{
		std::uint32_t m[16];

		for (int i = 0; i < 16; ++i)
		{
			const std::size_t o = block + static_cast<std::size_t>(i) * 4;
			m[i] = static_cast<std::uint32_t>(static_cast<unsigned char>(msg[o]))
				| (static_cast<std::uint32_t>(static_cast<unsigned char>(msg[o + 1])) << 8)
				| (static_cast<std::uint32_t>(static_cast<unsigned char>(msg[o + 2])) << 16)
				| (static_cast<std::uint32_t>(static_cast<unsigned char>(msg[o + 3])) << 24);
		}

		std::uint32_t a = a0, b = b0, c = c0, d = d0;

		for (int i = 0; i < 64; ++i)
		{
			std::uint32_t f;
			int g;

			if (i < 16)      { f = (b & c) | (~b & d);       g = i; }
			else if (i < 32) { f = (d & b) | (~d & c);       g = (5 * i + 1) % 16; }
			else if (i < 48) { f = b ^ c ^ d;                g = (3 * i + 5) % 16; }
			else             { f = c ^ (b | ~d);             g = (7 * i) % 16; }

			const std::uint32_t tmp = d;
			d = c;
			c = b;
			b = b + rotl32(a + f + MD5_K[i] + m[g], MD5_S[i]);
			a = tmp;
		}

		a0 += a;
		b0 += b;
		c0 += c;
		d0 += d;
	}

	String digest;
	digest.reserve(16);

	for (const std::uint32_t word : { a0, b0, c0, d0 })
		for (int i = 0; i < 4; ++i)
			digest += static_cast<char>((word >> (i * 8)) & 0xFFu);

	return cppString(std::move(digest));
}

sp::cppString sp::checksum::md5_hex(const cppString& data)
{
	const String digest = md5(data).s_str();

	String out;
	out.reserve(32);

	for (const char ch : digest)
	{
		const unsigned char byte = static_cast<unsigned char>(ch);
		out += "0123456789abcdef"[byte >> 4];
		out += "0123456789abcdef"[byte & 0xF];
	}

	return cppString(std::move(out));
}

namespace
{
	constexpr std::uint32_t SHA256_K[64] =
	{
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};

	constexpr std::uint32_t rotr32(std::uint32_t value, int count) noexcept
	{
		return (value >> count) | (value << (32 - count));
	}
}

sp::cppString sp::checksum::sha256(const cppString& data)
{
	const String& src = data.s_str();

	// Padded message: data + 0x80 + zeros + 64-bit big-endian bit length.
	String msg = src;
	msg += static_cast<char>(0x80);

	while (msg.size() % 64 != 56)
		msg += '\0';

	const std::uint64_t bit_len = static_cast<std::uint64_t>(src.size()) * 8u;

	for (int i = 7; i >= 0; --i)
		msg += static_cast<char>((bit_len >> (i * 8)) & 0xFFu);

	std::uint32_t h[8] =
	{
		0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
		0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
	};

	for (std::size_t block = 0; block < msg.size(); block += 64)
	{
		std::uint32_t w[64];

		for (int i = 0; i < 16; ++i)
		{
			const std::size_t o = block + static_cast<std::size_t>(i) * 4;
			w[i] = (static_cast<std::uint32_t>(static_cast<unsigned char>(msg[o])) << 24)
				| (static_cast<std::uint32_t>(static_cast<unsigned char>(msg[o + 1])) << 16)
				| (static_cast<std::uint32_t>(static_cast<unsigned char>(msg[o + 2])) << 8)
				| static_cast<std::uint32_t>(static_cast<unsigned char>(msg[o + 3]));
		}

		for (int i = 16; i < 64; ++i)
		{
			const std::uint32_t s0 = rotr32(w[i - 15], 7) ^ rotr32(w[i - 15], 18) ^ (w[i - 15] >> 3);
			const std::uint32_t s1 = rotr32(w[i - 2], 17) ^ rotr32(w[i - 2], 19) ^ (w[i - 2] >> 10);
			w[i] = w[i - 16] + s0 + w[i - 7] + s1;
		}

		std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
		std::uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

		for (int i = 0; i < 64; ++i)
		{
			const std::uint32_t s1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
			const std::uint32_t ch = (e & f) ^ (~e & g);
			const std::uint32_t t1 = hh + s1 + ch + SHA256_K[i] + w[i];
			const std::uint32_t s0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
			const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
			const std::uint32_t t2 = s0 + maj;

			hh = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		h[0] += a; h[1] += b; h[2] += c; h[3] += d;
		h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
	}

	String digest;
	digest.reserve(32);

	for (const std::uint32_t word : h)
		for (int i = 3; i >= 0; --i)
			digest += static_cast<char>((word >> (i * 8)) & 0xFFu);

	return cppString(std::move(digest));
}

sp::cppString sp::checksum::sha256_hex(const cppString& data)
{
	const String digest = sha256(data).s_str();

	String out;
	out.reserve(64);

	for (const char ch : digest)
	{
		const unsigned char byte = static_cast<unsigned char>(ch);
		out += "0123456789abcdef"[byte >> 4];
		out += "0123456789abcdef"[byte & 0xF];
	}

	return cppString(std::move(out));
}

// ---- HMAC-SHA-256 (RFC 2104) ----

sp::cppString sp::checksum::hmac_sha256(const cppString& key, const cppString& message)
{
	String k = key.s_str();

	if (k.size() > 64)
		k = sha256(cppString{ k }).s_str();

	k.resize(64, '\0');

	String inner;
	String outer;
	inner.reserve(64 + message.s_str().size());
	outer.reserve(64 + 32);

	for (const char ch : k)
	{
		const unsigned char byte = static_cast<unsigned char>(ch);
		inner += static_cast<char>(byte ^ 0x36u);
		outer += static_cast<char>(byte ^ 0x5Cu);
	}

	inner += message.s_str();
	outer += sha256(cppString{ inner }).s_str();

	return sha256(cppString{ outer });
}

sp::cppString sp::checksum::hmac_sha256_hex(const cppString& key, const cppString& message)
{
	const String digest = hmac_sha256(key, message).s_str();

	String out;
	out.reserve(64);

	for (const char ch : digest)
	{
		const unsigned char byte = static_cast<unsigned char>(ch);
		out += "0123456789abcdef"[byte >> 4];
		out += "0123456789abcdef"[byte & 0xF];
	}

	return cppString(std::move(out));
}
