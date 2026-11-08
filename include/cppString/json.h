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

#ifndef CPPSTRING_JSON_H_
#define CPPSTRING_JSON_H_

#include "cppString.h"

namespace sp
{
	// A minimal but strict JSON document model (RFC 8259).
	namespace json
	{
		enum class Type
		{
			Null,
			Bool,
			Number,
			String,
			Array,
			Object
		};

		// A JSON value. Objects preserve insertion order.
		// All as_* accessors throw Invalid_Operation on a type mismatch.
		class CPPSTRING_EXPORT Value
		{
		public:
			using array_t  = std::vector<Value>;
			using object_t = std::vector<std::pair<cppString, Value>>;

			Value();
			Value(std::nullptr_t);
			Value(bool value);
			Value(const char* value);
			Value(const cppString& value);
			Value(const array_t& value);
			Value(const object_t& value);

			// Any arithmetic type (except bool) becomes a Number.
			template<typename T,
				typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
			Value(T value) : m_type(Type::Number), m_number(static_cast<double>(value)) {}

			[[nodiscard]] Type type() const noexcept { return m_type; }

			[[nodiscard]] bool is_null()   const noexcept { return m_type == Type::Null; }
			[[nodiscard]] bool is_bool()   const noexcept { return m_type == Type::Bool; }
			[[nodiscard]] bool is_number() const noexcept { return m_type == Type::Number; }
			[[nodiscard]] bool is_string() const noexcept { return m_type == Type::String; }
			[[nodiscard]] bool is_array()  const noexcept { return m_type == Type::Array; }
			[[nodiscard]] bool is_object() const noexcept { return m_type == Type::Object; }

			// True for a Number with an integral value in the long long range.
			[[nodiscard]] bool is_integer() const noexcept;

			[[nodiscard]] bool as_bool() const;
			[[nodiscard]] double as_number() const;
			[[nodiscard]] long long as_int() const;      // throws unless is_integer()
			[[nodiscard]] const cppString& as_string() const;
			[[nodiscard]] const array_t& as_array() const;
			[[nodiscard]] const object_t& as_object() const;

			// Mutable access; a Null value is converted on demand.
			array_t& array_items();
			object_t& object_items();

			// Array access. Throws Invalid_Operation (not an array) or
			// Index_Out_Of_Range.
			const Value& operator[](std::size_t index) const;

			// Object access. Throws Invalid_Operation when not an object;
			// a missing key yields a shared Null value (never throws).
			const Value& operator[](const cppString& key) const;

			// True when this is an object containing key.
			[[nodiscard]] bool contains(const cppString& key) const noexcept;

			// Array/object: element count. String: UTF-8 character count.
			// Others: 0.
			[[nodiscard]] std::size_t size() const;

			// Serialize. indent < 0 gives compact form; otherwise pretty-print
			// with that many spaces per level. Keys keep insertion order.
			[[nodiscard]] cppString dump(int indent = -1) const;

			[[nodiscard]] bool operator==(const Value& other) const;
			[[nodiscard]] bool operator!=(const Value& other) const { return !(*this == other); }

		private:
			Type m_type = Type::Null;
			bool m_bool = false;
			double m_number = 0.0;
			cppString m_string;
			array_t m_array;
			object_t m_object;
		};

		// Parse a JSON document. Throws Invalid_Conversion with the byte
		// position on malformed input; trailing garbage is rejected.
		Value parse(const cppString& text);

		// RFC 6901 JSON Pointer. Tokens are '/'-separated with ~1 -> '/'
		// and ~0 -> '~'. The empty path denotes the whole document.
		// Throws Invalid_Operation / Index_Out_Of_Range when a token cannot
		// be resolved. Array tokens must be decimal indices.
		const Value& pointer(const Value& root, const cppString& path);

		// Set (or create) the value at a JSON Pointer path. Missing object
		// keys are appended; array indices are grown with Null values, and
		// "-" appends to an array. Throws Invalid_Conversion on a bad path.
		void set_pointer(Value& root, const cppString& path, Value value);

		// RFC 7386 JSON Merge Patch applied to a copy of target.
		// A Null patch member removes the key; objects merge recursively;
		// any other patch value replaces the target value.
		Value merge(Value target, const Value& patch);

		// RFC 6902 JSON Patch applied to a copy of document. Operations are
		// executed in order: add / remove / replace / move / copy / test.
		// Each op is an object with string "op" and "path"; "value" is
		// required for add/replace/test and "from" for move/copy.
		// Throws Invalid_Conversion on a malformed operation, and
		// Invalid_Operation / Index_Out_Of_Range when a path cannot be
		// resolved or a "test" comparison fails.
		Value patch(Value document, const Value& operations);
	}
}

#endif
