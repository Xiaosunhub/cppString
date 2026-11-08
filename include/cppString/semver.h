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

#ifndef CPPSTRING_SEMVER_H_
#define CPPSTRING_SEMVER_H_

#include "cppString.h"

namespace sp
{
	// Semantic Versioning 2.0.0 (semver.org): "MAJOR.MINOR.PATCH[-prerelease][+build]".
	namespace semver
	{
		// Strict validation per semver.org, including the "no leading zeros" rules.
		bool is_valid(const cppString& s) noexcept;

		// Precedence per semver.org; build metadata is ignored.
		// Returns < 0, 0 or > 0. Throws Invalid_Format when either side is not valid semver.
		int compare(const cppString& a, const cppString& b);

		// Bump one component and reset the lower ones; any prerelease or build
		// suffix is dropped ("1.2.3-rc.1+b5" -> bump_minor -> "1.3.0").
		// Throws Invalid_Format when the input is not valid semver.
		cppString bump_major(const cppString& s);
		cppString bump_minor(const cppString& s);
		cppString bump_patch(const cppString& s);
	}
}

#endif
