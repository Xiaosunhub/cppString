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

#include "table.h"
#include "utf8.h"

using sp::cppString;
using sp::table::Align;

namespace
{
	std::size_t width_of(const sp::cppString& cell)
	{
		return sp::utf8::display_width(cell);
	}

	String pad_cell(const String& text, std::size_t width, Align align)
	{
		const std::size_t w = width_of(sp::cppString{ text });

		if (w >= width)
			return text;

		const std::size_t gap = width - w;

		switch (align)
		{
		case Align::Right:
			return String(gap, ' ') + text;
		case Align::Center:
		{
			const std::size_t left = gap / 2;
			return String(left, ' ') + text + String(gap - left, ' ');
		}
		default:
			return text + String(gap, ' ');
		}
	}

	String border_of(const std::vector<std::size_t>& widths)
	{
		String border = "+";

		for (const std::size_t w : widths)
			border += String(w + 2, '-') + "+";

		return border;
	}

	void append_row(String& out, const std::vector<String>& cells,
		const std::vector<std::size_t>& widths, const std::vector<Align>& aligns)
	{
		out += '|';

		for (std::size_t i = 0; i < widths.size(); ++i)
		{
			const String cell = i < cells.size() ? cells[i] : String{};
			out += ' ';
			out += pad_cell(cell, widths[i], i < aligns.size() ? aligns[i] : Align::Left);
			out += " |";
		}

		out += '\n';
	}
}

cppString sp::table::render(const std::vector<std::vector<cppString>>& rows,
	const std::vector<cppString>& header,
	const std::vector<Align>& aligns)
{
	// Column count: the widest of header and rows.
	std::size_t cols = header.size();

	for (const auto& row : rows)
		cols = (std::max)(cols, row.size());

	if (cols == 0)
		return cppString{};

	std::vector<std::size_t> widths(cols, 0);

	auto measure = [&](const std::vector<cppString>& cells)
	{
		for (std::size_t i = 0; i < cells.size() && i < cols; ++i)
			widths[i] = (std::max)(widths[i], width_of(cells[i]));
	};

	measure(header);

	for (const auto& row : rows)
		measure(row);

	const String border = border_of(widths);

	String out;
	out.reserve(border.size() * (rows.size() + 3));

	out += border;
	out += '\n';

	if (!header.empty())
	{
		std::vector<String> cells;

		for (const auto& h : header)
			cells.push_back(h.s_str());

		append_row(out, cells, widths, aligns);
		out += border;
		out += '\n';
	}

	for (const auto& row : rows)
	{
		std::vector<String> cells;

		for (const auto& c : row)
			cells.push_back(c.s_str());

		append_row(out, cells, widths, aligns);
	}

	out += border;

	return cppString(std::move(out));
}
