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

#ifndef CPPSTRING_TABLE_H_
#define CPPSTRING_TABLE_H_

#include "cppString.h"

namespace sp
{
	// Plain-ASCII table rendering. Column widths are measured with
	// utf8::display_width, so CJK text stays aligned in a terminal.
	namespace table
	{
		enum class Align
		{
			Left,
			Center,
			Right
		};

		// Render rows with an optional header. Rows shorter than the widest
		// row are padded with empty cells; every cell keeps its own line
		// (embedded newlines are not expanded). Example:
		//
		//   +------+-----+
		//   | Name | Age |
		//   +------+-----+
		//   | Ann  |  12 |
		//   +------+-----+
		cppString render(const std::vector<std::vector<cppString>>& rows,
			const std::vector<cppString>& header = {},
			const std::vector<Align>& aligns = {});
	}
}

#endif
