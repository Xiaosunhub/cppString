// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Ender_

// ======================================================================
//    ___ ____  ____  ____ _____ ____  ___ _   _  ____
//   / __|  _ \|  _ \/ ___|_   _|  _ \|_ _| \ | |/ ___|
//  | (__| |_) | |_) \___ \ | | | |_) || ||  \| | |  _
//   \___| .__/|  __/ ___) || | |  _ < | || |\  | |_| |
//       |_|   |_|   |____/ |_| |_| \_\___|_| \_|\____|
//
//  C P P S T R I N G  -  a modern C++ string utility library
//  (formerly StringPlus / Twine / StringToolkit)
// ======================================================================
//
//
//Copyright(c)[2025][Ender_]
//
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files(the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and /or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions :
//
//The above copyright notice and this permission notice shall be included in all
//copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//SOFTWARE.

#ifndef CPPSTRING_CPPSTRING_H_
#define CPPSTRING_CPPSTRING_H_

#define CPPSTRING_VERSION_MAJOR 5
#define CPPSTRING_VERSION_MINOR 6
#define CPPSTRING_VERSION_PATCH 9
#define CPPSTRING_VERSION "5.6.9"

// Export control. A static library (the vcpkg default) needs no decoration;
// shared builds opt in with CPPSTRING_BUILD_SHARED / CPPSTRING_EXPORTS.
// Earlier releases hardcoded __declspec, which fails on non-MSVC toolchains.
#if defined(CPPSTRING_EXPORT)
    // A caller-supplied definition always wins.
#elif defined(CPPSTRING_BUILD_SHARED)
#   if defined(_WIN32)
#       if defined(CPPSTRING_EXPORTS)
#           define CPPSTRING_EXPORT __declspec(dllexport)
#       else
#           define CPPSTRING_EXPORT __declspec(dllimport)
#       endif
#   elif defined(__GNUC__) || defined(__clang__)
#       define CPPSTRING_EXPORT __attribute__((visibility("default")))
#   else
#       define CPPSTRING_EXPORT
#   endif
#else
#   define CPPSTRING_EXPORT
#endif

#include <iostream>
#include <string>
#include <vector>
#include <initializer_list>
#include <algorithm>
#include <cctype>
#include <memory>
#include <cstdio>
#include <utility>
#include <sstream>
#include <iomanip>
#include <regex>
#include <locale>
#include <functional>
#include <map>
#include <cstdlib>
#include <type_traits>
#include <format>
#include <set>
#include <exception>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <charconv>
#include <limits>
#include <fstream>

// std::basic_string / basic_string_view are already instantiated by the
// standard library; re-instantiating them here creates duplicate symbols on
// non-MSVC toolchains, so the library only aliases them (see below).


using String = std::basic_string<char, std::char_traits<char>, std::allocator<char>>;
using wString = std::basic_string<wchar_t, std::char_traits<wchar_t>, std::allocator<wchar_t>>;
using String_view = std::basic_string_view<char, std::char_traits<char>>;

namespace sp
{
    class [[nodiscard]] CPPSTRING_EXPORT String_Error : public std::exception
    {
    private:
        String message;

    public:
        explicit String_Error(const String& msg);
        explicit String_Error(const char* msg);

        [[nodiscard]] const char* what() const noexcept override;
    };

    class [[nodiscard]] CPPSTRING_EXPORT Index_Out_Of_Range : public sp::String_Error
    {
    public:
        explicit Index_Out_Of_Range(const String& msg);
        explicit Index_Out_Of_Range(const char* msg);
    };

    class [[nodiscard]] CPPSTRING_EXPORT Empty_String : public sp::String_Error
    {
    public:
        explicit Empty_String(const String& msg);
        explicit Empty_String(const char* msg);
    };

	class [[nodiscard]] CPPSTRING_EXPORT Invalid_Operation : public sp::String_Error
    {
    public:
        explicit Invalid_Operation(const String& msg);
        explicit Invalid_Operation(const char* msg);
	};

    class [[nodiscard]] CPPSTRING_EXPORT Invalid_Conversion : public sp::String_Error
    {
    public:
        explicit Invalid_Conversion(const String& msg);
        explicit Invalid_Conversion(const char* msg);
    };

	class [[nodiscard]] CPPSTRING_EXPORT Invalid_Argument : public sp::String_Error
    {
    public:
		explicit Invalid_Argument(const String& msg);
		explicit Invalid_Argument(const char* msg);
	};

    class [[nodiscard]] CPPSTRING_EXPORT Invalid_Format : public sp::String_Error
    {
    public:
        explicit Invalid_Format(const String & msg);
        explicit Invalid_Format(const char* msg);
    };

    class CPPSTRING_EXPORT cppString
    {
        String m_data{};

    public:
        using size_type              = std::size_t;
        using pos_type               = size_type;
        using value_type             = char;
        using iterator               = String::iterator;
        using const_iterator         = String::const_iterator;
        using reverse_iterator       = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;

        static constexpr auto npos{ static_cast<size_type>(-1) };

        friend std::istream& getline(std::istream& is, cppString& str, char delim);

        cppString();
        cppString(const String& _data);
        cppString(String&& _data);
        cppString(const char* _data);
        cppString(std::initializer_list<char> _data);
        cppString(std::vector<char> _data);
        cppString(std::set<char> _data);
        cppString(const cppString& _data);
        cppString(cppString&& _data) noexcept = default;
        cppString(String s, size_type size);
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> cppString(T num);
        template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> cppString(T num, int precision);
		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> cppString(std::vector<T> nums);
		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> cppString(std::initializer_list<T> nums);
		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>> cppString(std::set<T> nums);

        cppString& operator=(const cppString&)      = default;
        cppString& operator=(cppString&&)           noexcept = default;
        cppString& operator=(const String& s);
        cppString& operator=(String&& s)             noexcept;
        cppString& operator=(const char* cc);

        cppString create_from_this()                 const;

        const String& s_str()      const& noexcept;
        String&& s_str() &&        noexcept;
        const char* c_str()        const noexcept;
        const char* data()         const noexcept;
        char* data()               noexcept;
		String operator*()         const;
        String& get_changable_string();

        explicit operator String() const &;
        explicit operator String() &&;
        explicit operator const char*() const;
        explicit operator bool() const;

        bool empty()                      const noexcept;
        size_type size()                  const noexcept;
        size_type length()                const noexcept;
        size_type capacity()              const noexcept;
        void reserve(size_type new_cap);
        void shrink_to_fit();

        void clear()                      noexcept;

        cppString& insert(pos_type pos, const cppString& str);
        cppString& insert(pos_type pos, const String& str);
        cppString& insert(pos_type pos, const char* str);
        cppString& insert(pos_type pos, char ch);

        cppString& erase(pos_type pos = 0, pos_type count = npos);

        cppString& assign(const cppString& str);
        cppString& assign(const String& str);
        cppString& assign(const char* str);
        cppString& assign(pos_type count, char ch);

        void resize(size_type count);
        void resize(size_type count, char fill);

        int compare(const cppString& rhs) const;
        int compare(const String& rhs) const;
        int compare(const char* rhs) const;

        pos_type rfind(const cppString& s, pos_type pos = npos) const noexcept;
        pos_type rfind(const char* s, pos_type pos = npos)               const noexcept;
        pos_type rfind(char ch, pos_type pos = npos)                    const noexcept;

        cppString& append(const String& _data);
        cppString& append(const char* _data, size_type count);
        cppString& append(const char* _data);
        cppString& append(size_type count, char _data);

        template<class InputIt>
        cppString& append(InputIt first, InputIt last);

        cppString& operator+=(const cppString& rhs);
        cppString& operator+=(const String& rhs);
        cppString& operator+=(const char* rhs);
        cppString& operator+=(char ch);

        cppString operator+(const cppString& rhs) const;
        cppString operator+(const String& rhs) const;
        cppString operator+(const char* rhs) const;
        friend cppString operator+(const char* lhs, const cppString& rhs);
        friend cppString operator+(const String& lhs, const cppString& rhs);

        cppString operator*(size_type n) const;
        cppString& operator*=(size_type n);
		cppString operator-(size_type n) const;
		cppString& operator-=(size_type n);
        cppString& operator--();

        cppString& operator&=(const cppString& rhs);
        cppString& operator|=(const cppString& rhs);
        cppString& operator^=(const cppString& rhs);

        cppString operator&(const cppString& rhs)     const;
        cppString operator&(const String& rhs)    const;
        cppString operator&(const char* rhs)           const;

        cppString operator|(const cppString& rhs)     const;
        cppString operator|(const String& rhs)    const;
        cppString operator|(const char* rhs)           const;

        cppString operator^(const cppString& rhs)     const;
        cppString operator^(const String& rhs)    const;
        cppString operator^(const char* rhs)           const;

        cppString operator~()                          const = delete; 
        cppString operator<<(size_type)                const = delete;
        cppString operator>>(size_type)                const = delete;
        cppString& operator<<=(size_type)              = delete;
        cppString& operator>>=(size_type)              = delete;

        cppString& operator-=(char ch)                 = delete;
        cppString& operator-=(const cppString& rhs)   = delete;
        cppString& operator-=(const char* cc)          = delete;
        cppString& operator-=(const String& rhs)  = delete;

        cppString operator-(char ch)                   const = delete;
        cppString operator-(const cppString& rhs)     const = delete;
        cppString operator-(const char* cc)            const = delete;
        cppString operator-(const String& rhs)    const = delete;

        cppString& operator*=(char ch)                 = delete;
        cppString& operator*=(const cppString& rhs)   = delete;
        cppString& operator*=(const char* cc)          = delete;
        cppString& operator*=(const String& rhs)  = delete;

        cppString operator*(char ch)                   const = delete;
        cppString operator*(const cppString& rhs)     const = delete;
        cppString operator*(const char* cc)            const = delete;
        cppString operator*(const String& rhs)    const = delete;

        cppString& operator/=(char ch)                 = delete;
        cppString& operator/=(const cppString& rhs)   = delete;
        cppString& operator/=(const char* cc)          = delete;
        cppString& operator/=(const String& rhs)  = delete;

        cppString operator/(char ch)                   const = delete;
        cppString operator/(const cppString& rhs)     const = delete;
        cppString operator/(const char* cc)            const = delete;
        cppString operator/(const String& rhs)    const = delete;

        char& operator[](size_type pos);
        const char& operator[](size_type pos)           const;

        char& at(size_type pos);
        const char& at(size_type pos)                   const;

        char& front();
        const char& front()                             const;
        char& back();
        const char& back()                              const;

        iterator begin()                                noexcept;
        const_iterator begin()                          const noexcept;
        const_iterator cbegin()                         const noexcept;
        iterator end()                                  noexcept;
        const_iterator end()                            const noexcept;
        const_iterator cend()                           const noexcept;
        reverse_iterator rbegin()                       noexcept;
        const_reverse_iterator rbegin()                 const noexcept;
        const_reverse_iterator crbegin()                const noexcept;
        reverse_iterator rend()                         noexcept;
        const_reverse_iterator rend()                   const noexcept;
        const_reverse_iterator crend()                  const noexcept;

        auto operator<=>(cppString rhs) const { return m_data <=> rhs.m_data; }
        bool operator==(cppString rhs) const { return m_data == rhs.m_data; }

        cppString substr(pos_type pos = 0, pos_type count = npos) const;
        // Substring limited to [start_pos, end_pos); count == npos means "up to end_pos".
        // Named differently so a two-argument call is never ambiguous with substr(pos, count).
        cppString substr_between(pos_type start_pos, pos_type end_pos, pos_type count = npos) const;

        pos_type find(const cppString& s, pos_type pos = 0) const noexcept;
        pos_type find(const char* s, pos_type pos = 0)       const noexcept;
        pos_type find(char ch, pos_type pos = 0)             const noexcept;

		cppString& replace(pos_type pos, pos_type count, const cppString& str);
		cppString& replace(pos_type pos, pos_type count, const char* s);
		cppString& replace(pos_type pos, pos_type count, const String& str);

        cppString& toUpper();
		cppString& toLower();
        cppString copyUpper() const;
		cppString copyLower() const;

		cppString& trimLeft();
		cppString& trimRight();
		cppString& trim();
		cppString& trim(char ch);
        cppString copyTrim() const;
        cppString copyTrim(char ch) const;

        cppString& replaceAll(const cppString& from, const cppString& to);
		cppString copyReplaceAll(const cppString& from, const cppString& to);

		bool startsWith(const cppString& prefix) const;
		bool startsWith(const char* prefix) const;
		bool startsWith(const String& prefix) const;
        bool startsWith(char ch) const;
        bool endsWith(const cppString& suffix) const;
		bool endsWith(const char* suffix) const;
		bool endsWith(const String& suffix) const;
		bool endsWith(char ch) const;

		bool contains(const cppString& subString) const;
		bool contains(const char* subString) const;
		bool contains(const String& subString) const;
		bool contains(char ch) const;
		bool contains(const std::initializer_list<char>& chars) const;
		bool contains(const std::vector<char>& chars) const;
		bool contains(const std::set<char>& chars) const;
		bool contains(const std::initializer_list<String>& subStrings) const;
		bool contains(const std::vector<String>& subStrings) const;
		bool contains(const std::set<String>& subStrings) const;
		bool contains(const std::initializer_list<cppString>& subStrings) const;
		bool contains(const std::vector<cppString>& subStrings) const;
		bool contains(const std::set<cppString>& subStrings) const;
		bool contains(const std::initializer_list<const char*>& subStrings) const;
		bool contains(const std::vector<const char*>& subStrings) const;
		bool contains(const std::set<const char*>& subStrings) const;
		bool contains(const std::initializer_list<String_view>& subStrings) const;
		bool contains(const std::vector<String_view>& subStrings) const;
		bool contains(const std::set<String_view>& subStrings) const;

		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        cppString& fromNumber(T num);
        
		template<typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
		T toNumber() const;

        template <typename... Args>
		cppString& format(Args&&... args);

		template <typename... Args>
		cppString concat(Args&&... args);

        void push_back(char c);
        void pop_back();

        bool matches(const cppString& pattern) const;
		bool matches(const char* pattern) const;
		bool matches(const String& pattern) const;

		void regexReplace(const cppString& pattern, const cppString& replacement);
		void regexReplace(const char* pattern, const char* replacement);
		void regexReplace(const String& pattern, const String& replacement);

		cppString& fromWide(const wString& wideStr);
		wString toWide() const;

		std::vector<cppString> split(const cppString& delimiter) const;
		std::vector<cppString> split(const char* delimiter) const;
		std::vector<cppString> split(const String& delimiter) const;
		std::vector<cppString> split(char delimiter) const;

		size_type findFirstOf(const cppString& chars, pos_type pos = 0) const;
		size_type findFirstOf(const char* chars, pos_type pos = 0) const;
		size_type findFirstOf(const String& chars, pos_type pos = 0) const;
		size_type findFirstOf(char ch, pos_type pos = 0) const;
		size_type findLastOf(const cppString& chars, pos_type pos = npos) const;
		size_type findLastOf(const char* chars, pos_type pos = npos) const;
		size_type findLastOf(const String& chars, pos_type pos = npos) const;
		size_type findLastOf(char ch, pos_type pos = npos) const;

		size_type find_first_not_of(const cppString& chars, pos_type pos = 0) const noexcept;
		size_type find_first_not_of(const char* chars, pos_type pos = 0) const noexcept;
		size_type find_first_not_of(char ch, pos_type pos = 0) const noexcept;
		size_type find_last_not_of(const cppString& chars, pos_type pos = npos) const noexcept;
		size_type find_last_not_of(const char* chars, pos_type pos = npos) const noexcept;
		size_type find_last_not_of(char ch, pos_type pos = npos) const noexcept;

		size_type UTF8Length() const;
		size_type UTF8CharCount() const;

		cppString UTF8SubString(size_type start, size_type length) const;
		cppString UTF8SubString(size_type start) const;
		cppString UTF8SubString(size_type start, size_type end, size_type length) const;

		void swap(cppString& other) noexcept;

        bool is_clear() const noexcept;
		bool is_empty() const noexcept;
		bool is_not_empty() const noexcept;
		bool is_not_clear() const noexcept;
		bool is_whitespace() const noexcept;
        bool is_not_whitespace() const noexcept;
		bool is_digit() const noexcept;
        bool is_not_digit() const noexcept;
		bool is_alpha() const noexcept;
		bool is_not_alpha() const noexcept;
		bool is_alnum() const noexcept;
		bool is_not_alnum() const noexcept;
		bool is_lower() const noexcept;
		bool is_not_lower() const noexcept;
		bool is_upper() const noexcept;
		bool is_not_upper() const noexcept;
		bool is_ascii() const noexcept;
		bool is_not_ascii() const noexcept;
		bool is_printable() const noexcept;
		bool is_not_printable() const noexcept;
		bool is_graphical() const noexcept;
		bool is_not_graphical() const noexcept;
		bool is_punctuation() const noexcept;
		bool is_not_punctuation() const noexcept;
		bool is_control() const noexcept;
		bool is_not_control() const noexcept;
		bool is_hex() const noexcept;
		bool is_not_hex() const noexcept;
		bool is_base64() const noexcept;
		bool is_not_base64() const noexcept;
		bool is_utf8() const noexcept;
		bool is_not_utf8() const noexcept;
		bool is_regex_string() const noexcept;
		bool is_not_regex_string() const noexcept;

		void reset(const String& _data);
		void reset(String&& _data) noexcept;
		void reset(const char* _data) noexcept;
		void reset(const cppString& _data);
		void reset(cppString&& _data) noexcept;
        void reset();

		void reverse();

		size_type count(char ch) const;
		size_type count(const cppString& str) const;
		size_type count(String_view str) const;

        void join(cppString data);
		void join(const String& data);
		void join(const char* data);
		void join(char data);

        template<typename T>
		void remove_prefix(const T& prefix);
        template<typename T>
		void remove_suffix(const T& suffix);

        static cppString create();

        template<typename Func>
        auto and_then(Func&& func)
        {
            return std::invoke(
                std::forward<Func>(func),
                std::forward<sp::cppString>(*this)
            );
        }

        ~cppString()                                   = default;
    };

    // ---- Template member definitions (must live in the header for implicit instantiation) ----

    template<typename T, typename>
    cppString::cppString(T num) : m_data(std::to_string(num)) {}

    template<typename T, typename>
    cppString::cppString(T num, int precision)
    {
        m_data = std::format("{:.{}f}", num, precision);
    }

    template<typename T, typename>
    cppString::cppString(std::vector<T> nums)
    {
        for (auto& num : nums) { m_data += static_cast<char>(num); }
    }

    template<typename T, typename>
    cppString::cppString(std::initializer_list<T> nums)
    {
        for (auto& num : nums) { m_data += static_cast<char>(num); }
    }

    template<typename T, typename>
    cppString::cppString(std::set<T> nums)
    {
        for (auto& num : nums) { m_data += static_cast<char>(num); }
    }

    template<class InputIt>
    cppString& cppString::append(InputIt first, InputIt last)
    {
        m_data.append(first, last);
        return *this;
    }

    template<typename T, typename>
    cppString& cppString::fromNumber(T num)
    {
        m_data = std::to_string(num);
        return *this;
    }

    template<typename T, typename>
    T cppString::toNumber() const
    {
        if (m_data.empty()) {
            throw sp::Invalid_Conversion("Cannot convert empty string to number.");
        }

        if constexpr (std::is_integral_v<T>) {
            T value{};

            const char* begin = m_data.data();
            const char* end = m_data.data() + m_data.size();

            auto [position, error] = std::from_chars(begin, end, value);

            if (error != std::errc{} || position != end) {
                throw sp::Invalid_Conversion("Invalid conversion from string to number.");
            }

            return value;
        }
        else if constexpr (std::is_floating_point_v<T>) {
            String buffer(m_data);

            char* end = nullptr;
            errno = 0;

            long double value = std::strtold(buffer.c_str(), &end);

            if (end == buffer.c_str() || *end != '\0') {
                throw sp::Invalid_Conversion("Invalid conversion from string to number.");
            }

            if (errno == ERANGE) {
                throw sp::Invalid_Conversion("Conversion from string to number is out of range.");
            }

            // check target type range
            if (value > std::numeric_limits<T>::max() ||
                value < std::numeric_limits<T>::lowest()) {
                throw sp::Invalid_Conversion("Conversion from string to number is out of range for the target type.");
            }

            return static_cast<T>(value);
        }
    }

    template<typename... Args>
    cppString& cppString::format(Args&&... args)
    {
        const int formattedSize = std::snprintf(
            nullptr,
            0,
            m_data.c_str(),
            std::forward<Args>(args)...
        );

        if (formattedSize < 0) {
            throw sp::Invalid_Format("Error during formatting.");
        }

        const std::size_t bufferSize =
            static_cast<std::size_t>(formattedSize) + 1;

        auto buffer = std::make_unique<char[]>(bufferSize);

        const int written = std::snprintf(
            buffer.get(),
            bufferSize,
            m_data.c_str(),
            std::forward<Args>(args)...
        );

        if (written < 0 || written != formattedSize) {
            throw sp::Invalid_Format("Error during formatting.");
        }

        m_data.assign(buffer.get(), static_cast<std::size_t>(written));

        return *this;
    }

    template<typename... Args>
    cppString cppString::concat(Args&&... args)
    {
        return (cppString{} + ... + cppString(std::forward<Args>(args)));
    }

    template<typename T>
    void cppString::remove_prefix(const T& prefix)
    {
        String prefix_str;
        if constexpr (std::is_constructible_v<String_view, const T&>)
            prefix_str = String(String_view(prefix));
        else
            prefix_str = prefix.s_str();

        if (prefix_str.size() > m_data.size())
            throw sp::Invalid_Argument("Prefix is longer than the string in remove_prefix.");

        if (!startsWith(prefix_str))
            throw sp::Invalid_Argument("String does not start with the specified prefix in remove_prefix.");

        m_data.erase(0, prefix_str.size());
    }

    template<typename T>
    void cppString::remove_suffix(const T& suffix)
    {
        String suffix_str;
        if constexpr (std::is_constructible_v<String_view, const T&>)
            suffix_str = String(String_view(suffix));
        else
            suffix_str = suffix.s_str();

        if (suffix_str.size() > m_data.size())
            throw sp::Invalid_Argument("Suffix is longer than the string in remove_suffix.");

        if (!endsWith(suffix_str))
            throw sp::Invalid_Argument("String does not end with the specified suffix in remove_suffix.");

        m_data.erase(m_data.size() - suffix_str.size(), suffix_str.size());
    }

    std::istream& getline(std::istream& is, cppString& str, char delim = '\n');

    // Enables std::swappable (ADL): required for std::movable / ranges algorithms.
    void swap(cppString& lhs, cppString& rhs) noexcept;

	cppString operator""_sp(const char* data, size_t size);
}

using std::istream, std::ostream;

istream& operator>>(istream& is, sp::cppString& str);
ostream& operator<<(ostream& os, const sp::cppString& str);

namespace std
{
    template<>
    struct std::formatter<sp::cppString, char>
        : std::formatter<String_view, char>
    {
        constexpr auto parse(std::format_parse_context& ctx)
        {
            return std::formatter<String_view, char>::parse(ctx);
        }

        auto format(const sp::cppString& value,
            std::format_context& ctx) const
        {
            return std::formatter<String_view, char>::format(
                value.s_str(),
                ctx
            );
        }
    };
}

namespace std {
    template<>
    struct hash<sp::cppString> {
        size_t operator()(const sp::cppString& s) const noexcept {
            return std::hash<std::string_view>{}(std::string_view(s.c_str(), s.size()));
        }
    };
}

#endif