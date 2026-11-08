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

#ifndef CPPSTRING_CHECKSUM_H_
#define CPPSTRING_CHECKSUM_H_

#include "cppString.h"
#include <cstdint>

namespace sp
{
	// Non-cryptographic checksums for integrity checks and cache keys.
	// None of these is a substitute for a cryptographic hash.
	namespace checksum
	{
		// CRC-32/ISO-HDLC (the zlib / PNG variant).
		std::uint32_t crc32(const cppString& data) noexcept;
		cppString crc32_hex(const cppString& data);          // 8 lowercase hex digits

		// FNV-1a, 64-bit.
		std::uint64_t fnv1a64(const cppString& data) noexcept;
		cppString fnv1a64_hex(const cppString& data);        // 16 lowercase hex digits

		// Adler-32 (the zlib rolling checksum).
		std::uint32_t adler32(const cppString& data) noexcept;
		cppString adler32_hex(const cppString& data);        // 8 lowercase hex digits

		// MD5 (RFC 1321). Legacy digest: fine for cache keys and
		// fingerprints, NOT collision-resistant - never use for security.
		cppString md5(const cppString& data);                // 16 raw digest bytes
		cppString md5_hex(const cppString& data);            // 32 lowercase hex digits

		// SHA-256 (FIPS 180-4). Collision-resistant digest suitable for
		// integrity checks and content addressing.
		cppString sha256(const cppString& data);             // 32 raw digest bytes
		cppString sha256_hex(const cppString& data);         // 64 lowercase hex digits

		// HMAC-SHA-256 (RFC 2104). Keys longer than the 64-byte block size
		// are hashed first; short keys are zero-padded.
		cppString hmac_sha256(const cppString& key, const cppString& message);
		cppString hmac_sha256_hex(const cppString& key, const cppString& message);
	}
}

#endif
