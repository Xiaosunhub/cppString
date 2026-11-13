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

#ifndef CPPSTRING_CSV_H_
#define CPPSTRING_CSV_H_

#include "cppString.h"
#include <vector>
#include <utility>
#include <cstddef>

namespace sp
{
	namespace json { class Value; }

	// Full RFC 4180 document support. This extends the single-line
	// helpers sp::csv::escape / sp::csv::split declared in method.h.
	namespace csv
	{
		class CPPSTRING_EXPORT Document
		{
		public:
			using Row = std::vector<cppString>;

			Document() = default;
			explicit Document(Row header);

			[[nodiscard]] bool has_header() const noexcept { return m_has_header; }
			[[nodiscard]] const Row& header() const noexcept;
			void set_header(Row header);
			void clear_header() noexcept;

			[[nodiscard]] const std::vector<Row>& rows() const noexcept { return m_rows; }
			[[nodiscard]] std::vector<Row>& rows() noexcept { return m_rows; }
			void add_row(Row row);

			[[nodiscard]] std::size_t row_count() const noexcept { return m_rows.size(); }
			[[nodiscard]] std::size_t column_count() const noexcept;

			// Throws Index_Out_Of_Range on a bad row index.
			[[nodiscard]] const Row& operator[](std::size_t row) const;

			// Cell access. Missing cells yield an empty string.
			// get(row, column_name) throws Invalid_Argument when there is no
			// header or the column is unknown.
			[[nodiscard]] cppString at(std::size_t row, std::size_t column) const;
			[[nodiscard]] cppString get(std::size_t row, const cppString& column) const;

			// Index of a header column, or npos when absent.
			[[nodiscard]] std::size_t column_index(const cppString& column) const noexcept;

		private:
			Row m_header{};
			bool m_has_header = false;
			std::vector<Row> m_rows{};
		};

		// Guess the separator from the first record, counting occurrences
		// outside quoted fields. Candidates: ',' ';' '\t' '|'. Falls back to ','.
		[[nodiscard]] char sniff_delimiter(const cppString& text) noexcept;

		// Parse a whole document. Handles quoted fields containing the
		// separator, doubled quotes and embedded line breaks; CRLF / CR
		// inside a quoted field is preserved verbatim so write()->parse()
		// round-trips exactly. A blank line is a record with zero fields,
		// while a trailing line break does not add an extra record. Throws
		// Invalid_Conversion on an unterminated quote. Pass delimiter '\0'
		// to auto-detect.
		[[nodiscard]] Document parse(const cppString& text, char delimiter = '\0',
			bool has_header = true);

		// Serialize. Fields are quoted only when needed (or always when
		// always_quote is set). Every record, including the last, is
		// terminated by line_ending.
		[[nodiscard]] cppString write(const Document& doc, char delimiter = ',',
			bool always_quote = false, const cppString& line_ending = "\r\n");

		[[nodiscard]] std::vector<Document::Row> parse_rows(const cppString& text,
			char delimiter = '\0');
		[[nodiscard]] cppString write_rows(const std::vector<Document::Row>& rows,
			char delimiter = ',', const cppString& line_ending = "\r\n");

		// ---- JSON interop ----
		// Cells are always JSON strings, so a to_json -> from_json round trip
		// is lossless. With named=true and a header, to_json produces an array
		// of objects keyed by the header; otherwise it produces an array of
		// arrays (the header row included first when present).
		[[nodiscard]] json::Value to_json(const Document& doc, bool named = true);

		// Inverse of to_json. with named=true the first object's keys become
		// the header (in order); keys appearing only in later rows extend it.
		// Non-string values are serialized with json::Value::dump; Null becomes
		// an empty cell. Throws Invalid_Conversion on an unexpected shape.
		[[nodiscard]] Document from_json(const json::Value& value, bool named = true);
	}
}

#endif
