// The MIT License (MIT)
//
// Copyright (c) 2013-2015 Niels Lohmann
// Copyright (c) 2017 THINK BIG LABS S.L.
// Author: alvarolb@gmail.com (Alvaro Luis Bustamante)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef JSON_DECODER_HPP
#define JSON_DECODER_HPP

#include <stdexcept>
#include <string>
#include <stdlib.h>
#include <cmath>
#include "../pson.h"

namespace protoson {

    using string_t = std::string;
    using boolean_t = bool;
    using number_integer_t = int64_t;
    using number_float_t = double;

    class lexer {
    public:
        /// token types for the parser
        enum class token_type {
            uninitialized,    ///< indicating the scanner is uninitialized
            literal_true,     ///< the "true" literal
            literal_false,    ///< the "false" literal
            literal_null,     ///< the "null" literal
            value_string,     ///< a string - use get_string() for actual value
            value_number,     ///< a number - use get_number() for actual value
            begin_array,      ///< the character for array begin "["
            begin_object,     ///< the character for object begin "{"
            end_array,        ///< the character for array end "]"
            end_object,       ///< the character for object end "}"
            name_separator,   ///< the name separator ":"
            value_separator,  ///< the value separator ","
            parse_error,      ///< indicating a parse error
            end_of_input      ///< indicating the end of the input buffer
        };

        /// the char type to use in the lexer
        using lexer_char_t = unsigned char;

        /// constructor with a given buffer
        inline lexer(const string_t &s) noexcept
                : m_content(reinterpret_cast<const lexer_char_t *>(s.c_str())) {
            m_start = m_cursor = m_content;
            m_limit = m_content + s.size();
        }

        /// default constructor
        inline lexer() = default;

        inline static string_t to_unicode(const size_t codepoint1, const size_t codepoint2 = 0) {
            string_t result;

            // calculate the codepoint from the given code points
            size_t codepoint = codepoint1;
            if (codepoint1 >= 0xD800 and codepoint1 <= 0xDBFF) {
                if (codepoint2 >= 0xDC00 and codepoint2 <= 0xDFFF) {
                    codepoint =
                            // high surrogate occupies the most significant 22 bits
                            (codepoint1 << 10)
                                    // low surrogate occupies the least significant 15 bits
                                    + codepoint2
                                    // there is still the 0xD800, 0xDC00 and 0x10000 noise
                                    // in the result so we have to substract with:
                                    // (0xD800 << 10) + DC00 - 0x10000 = 0x35FDC00
                                    - 0x35FDC00;
                }
                else {
                    throw std::invalid_argument("missing or wrong low surrogate");
                }
            }

            if (codepoint <= 0x7f) {
                // 1-byte characters: 0xxxxxxx (ASCI)
                result.append(1, static_cast<typename string_t::value_type>(codepoint));
            }
            else if (codepoint <= 0x7ff) {
                // 2-byte characters: 110xxxxx 10xxxxxx
                result.append(1, static_cast<typename string_t::value_type>(0xC0 | ((codepoint >> 6) & 0x1F)));
                result.append(1, static_cast<typename string_t::value_type>(0x80 | (codepoint & 0x3F)));
            }
            else if (codepoint <= 0xffff) {
                // 3-byte characters: 1110xxxx 10xxxxxx 10xxxxxx
                result.append(1, static_cast<typename string_t::value_type>(0xE0 | ((codepoint >> 12) & 0x0F)));
                result.append(1, static_cast<typename string_t::value_type>(0x80 | ((codepoint >> 6) & 0x3F)));
                result.append(1, static_cast<typename string_t::value_type>(0x80 | (codepoint & 0x3F)));
            }
            else if (codepoint <= 0x10ffff) {
                // 4-byte characters: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
                result.append(1, static_cast<typename string_t::value_type>(0xF0 | ((codepoint >> 18) & 0x07)));
                result.append(1, static_cast<typename string_t::value_type>(0x80 | ((codepoint >> 12) & 0x3F)));
                result.append(1, static_cast<typename string_t::value_type>(0x80 | ((codepoint >> 6) & 0x3F)));
                result.append(1, static_cast<typename string_t::value_type>(0x80 | (codepoint & 0x3F)));
            }
            else {
                throw std::out_of_range("code points above 0x10FFFF are invalid");
            }

            return result;
        }

        inline static std::string token_type_name(token_type t) noexcept {
            switch (t) {
                case (token_type::uninitialized):
                    return "<uninitialized>";
                case (token_type::literal_true):
                    return "true literal";
                case (token_type::literal_false):
                    return "false literal";
                case (token_type::literal_null):
                    return "null literal";
                case (token_type::value_string):
                    return "string literal";
                case (token_type::value_number):
                    return "number literal";
                case (token_type::begin_array):
                    return "[";
                case (token_type::begin_object):
                    return "{";
                case (token_type::end_array):
                    return "]";
                case (token_type::end_object):
                    return "}";
                case (token_type::name_separator):
                    return ":";
                case (token_type::value_separator):
                    return ",";
                case (token_type::end_of_input):
                    return "<end of input>";
                default:
                    return "<parse error>";
            }
        }

        inline token_type scan() noexcept {
            // pointer for backtracking information
            const lexer_char_t *m_marker = nullptr;

            // remember the begin of the token
            m_start = m_cursor;


            {
                lexer_char_t yych;
                unsigned int yyaccept = 0;
                static const unsigned char yybm[] =
                        {
                                0, 64, 64, 64, 64, 64, 64, 64,
                                64, 96, 96, 64, 64, 96, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                96, 64, 0, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                192, 192, 192, 192, 192, 192, 192, 192,
                                192, 192, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 0, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                                64, 64, 64, 64, 64, 64, 64, 64,
                        };

                yych = *m_cursor;
                if (yych <= '9') {
                    if (yych <= ' ') {
                        if (yych <= '\n') {
                            if (yych <= 0x00) {
                                goto basic_json_parser_27;
                            }
                            if (yych <= 0x08) {
                                goto basic_json_parser_29;
                            }
                            if (yych >= '\n') {
                                goto basic_json_parser_4;
                            }
                        }
                        else {
                            if (yych == '\r') {
                                goto basic_json_parser_2;
                            }
                            if (yych <= 0x1F) {
                                goto basic_json_parser_29;
                            }
                        }
                    }
                    else {
                        if (yych <= ',') {
                            if (yych == '"') {
                                goto basic_json_parser_26;
                            }
                            if (yych <= '+') {
                                goto basic_json_parser_29;
                            }
                            goto basic_json_parser_14;
                        }
                        else {
                            if (yych <= '-') {
                                goto basic_json_parser_22;
                            }
                            if (yych <= '/') {
                                goto basic_json_parser_29;
                            }
                            if (yych <= '0') {
                                goto basic_json_parser_23;
                            }
                            goto basic_json_parser_25;
                        }
                    }
                }
                else {
                    if (yych <= 'm') {
                        if (yych <= '\\') {
                            if (yych <= ':') {
                                goto basic_json_parser_16;
                            }
                            if (yych == '[') {
                                goto basic_json_parser_6;
                            }
                            goto basic_json_parser_29;
                        }
                        else {
                            if (yych <= ']') {
                                goto basic_json_parser_8;
                            }
                            if (yych == 'f') {
                                goto basic_json_parser_21;
                            }
                            goto basic_json_parser_29;
                        }
                    }
                    else {
                        if (yych <= 'z') {
                            if (yych <= 'n') {
                                goto basic_json_parser_18;
                            }
                            if (yych == 't') {
                                goto basic_json_parser_20;
                            }
                            goto basic_json_parser_29;
                        }
                        else {
                            if (yych <= '{') {
                                goto basic_json_parser_10;
                            }
                            if (yych == '}') {
                                goto basic_json_parser_12;
                            }
                            goto basic_json_parser_29;
                        }
                    }
                }
                basic_json_parser_2:
                ++m_cursor;
                yych = *m_cursor;
                goto basic_json_parser_5;
                basic_json_parser_3:
                {
                    return scan();
                }
                basic_json_parser_4:
                ++m_cursor;
                yych = *m_cursor;
                basic_json_parser_5:
                if (yybm[0 + yych] & 32) {
                    goto basic_json_parser_4;
                }
                goto basic_json_parser_3;
                basic_json_parser_6:
                ++m_cursor;
                {
                    return token_type::begin_array;
                }
                basic_json_parser_8:
                ++m_cursor;
                {
                    return token_type::end_array;
                }
                basic_json_parser_10:
                ++m_cursor;
                {
                    return token_type::begin_object;
                }
                basic_json_parser_12:
                ++m_cursor;
                {
                    return token_type::end_object;
                }
                basic_json_parser_14:
                ++m_cursor;
                {
                    return token_type::value_separator;
                }
                basic_json_parser_16:
                ++m_cursor;
                {
                    return token_type::name_separator;
                }
                basic_json_parser_18:
                yyaccept = 0;
                yych = *(m_marker = ++m_cursor);
                if (yych == 'u') {
                    goto basic_json_parser_59;
                }
                basic_json_parser_19:
                {
                    return token_type::parse_error;
                }
                basic_json_parser_20:
                yyaccept = 0;
                yych = *(m_marker = ++m_cursor);
                if (yych == 'r') {
                    goto basic_json_parser_55;
                }
                goto basic_json_parser_19;
                basic_json_parser_21:
                yyaccept = 0;
                yych = *(m_marker = ++m_cursor);
                if (yych == 'a') {
                    goto basic_json_parser_50;
                }
                goto basic_json_parser_19;
                basic_json_parser_22:
                yych = *++m_cursor;
                if (yych <= '/') {
                    goto basic_json_parser_19;
                }
                if (yych <= '0') {
                    goto basic_json_parser_49;
                }
                if (yych <= '9') {
                    goto basic_json_parser_40;
                }
                goto basic_json_parser_19;
                basic_json_parser_23:
                yyaccept = 1;
                yych = *(m_marker = ++m_cursor);
                if (yych <= 'D') {
                    if (yych == '.') {
                        goto basic_json_parser_42;
                    }
                }
                else {
                    if (yych <= 'E') {
                        goto basic_json_parser_43;
                    }
                    if (yych == 'e') {
                        goto basic_json_parser_43;
                    }
                }
                basic_json_parser_24:
                {
                    return token_type::value_number;
                }
                basic_json_parser_25:
                yyaccept = 1;
                yych = *(m_marker = ++m_cursor);
                goto basic_json_parser_41;
                basic_json_parser_26:
                yyaccept = 0;
                yych = *(m_marker = ++m_cursor);
                if (yych <= 0x00) {
                    goto basic_json_parser_19;
                }
                goto basic_json_parser_31;
                basic_json_parser_27:
                ++m_cursor;
                {
                    return token_type::end_of_input;
                }
                basic_json_parser_29:
                yych = *++m_cursor;
                goto basic_json_parser_19;
                basic_json_parser_30:
                ++m_cursor;
                yych = *m_cursor;
                basic_json_parser_31:
                if (yybm[0 + yych] & 64) {
                    goto basic_json_parser_30;
                }
                if (yych <= 0x00) {
                    goto basic_json_parser_32;
                }
                if (yych <= '"') {
                    goto basic_json_parser_34;
                }
                goto basic_json_parser_33;
                basic_json_parser_32:
                m_cursor = m_marker;
                if (yyaccept == 0) {
                    goto basic_json_parser_19;
                }
                else {
                    goto basic_json_parser_24;
                }
                basic_json_parser_33:
                ++m_cursor;
                yych = *m_cursor;
                if (yych <= 'e') {
                    if (yych <= '/') {
                        if (yych == '"') {
                            goto basic_json_parser_30;
                        }
                        if (yych <= '.') {
                            goto basic_json_parser_32;
                        }
                        goto basic_json_parser_30;
                    }
                    else {
                        if (yych <= '\\') {
                            if (yych <= '[') {
                                goto basic_json_parser_32;
                            }
                            goto basic_json_parser_30;
                        }
                        else {
                            if (yych == 'b') {
                                goto basic_json_parser_30;
                            }
                            goto basic_json_parser_32;
                        }
                    }
                }
                else {
                    if (yych <= 'q') {
                        if (yych <= 'f') {
                            goto basic_json_parser_30;
                        }
                        if (yych == 'n') {
                            goto basic_json_parser_30;
                        }
                        goto basic_json_parser_32;
                    }
                    else {
                        if (yych <= 's') {
                            if (yych <= 'r') {
                                goto basic_json_parser_30;
                            }
                            goto basic_json_parser_32;
                        }
                        else {
                            if (yych <= 't') {
                                goto basic_json_parser_30;
                            }
                            if (yych <= 'u') {
                                goto basic_json_parser_36;
                            }
                            goto basic_json_parser_32;
                        }
                    }
                }
                basic_json_parser_34:
                ++m_cursor;
                {
                    return token_type::value_string;
                }
                basic_json_parser_36:
                ++m_cursor;
                yych = *m_cursor;
                if (yych <= '@') {
                    if (yych <= '/') {
                        goto basic_json_parser_32;
                    }
                    if (yych >= ':') {
                        goto basic_json_parser_32;
                    }
                }
                else {
                    if (yych <= 'F') {
                        goto basic_json_parser_37;
                    }
                    if (yych <= '`') {
                        goto basic_json_parser_32;
                    }
                    if (yych >= 'g') {
                        goto basic_json_parser_32;
                    }
                }
                basic_json_parser_37:
                ++m_cursor;
                yych = *m_cursor;
                if (yych <= '@') {
                    if (yych <= '/') {
                        goto basic_json_parser_32;
                    }
                    if (yych >= ':') {
                        goto basic_json_parser_32;
                    }
                }
                else {
                    if (yych <= 'F') {
                        goto basic_json_parser_38;
                    }
                    if (yych <= '`') {
                        goto basic_json_parser_32;
                    }
                    if (yych >= 'g') {
                        goto basic_json_parser_32;
                    }
                }
                basic_json_parser_38:
                ++m_cursor;
                yych = *m_cursor;
                if (yych <= '@') {
                    if (yych <= '/') {
                        goto basic_json_parser_32;
                    }
                    if (yych >= ':') {
                        goto basic_json_parser_32;
                    }
                }
                else {
                    if (yych <= 'F') {
                        goto basic_json_parser_39;
                    }
                    if (yych <= '`') {
                        goto basic_json_parser_32;
                    }
                    if (yych >= 'g') {
                        goto basic_json_parser_32;
                    }
                }
                basic_json_parser_39:
                ++m_cursor;
                yych = *m_cursor;
                if (yych <= '@') {
                    if (yych <= '/') {
                        goto basic_json_parser_32;
                    }
                    if (yych <= '9') {
                        goto basic_json_parser_30;
                    }
                    goto basic_json_parser_32;
                }
                else {
                    if (yych <= 'F') {
                        goto basic_json_parser_30;
                    }
                    if (yych <= '`') {
                        goto basic_json_parser_32;
                    }
                    if (yych <= 'f') {
                        goto basic_json_parser_30;
                    }
                    goto basic_json_parser_32;
                }
                basic_json_parser_40:
                yyaccept = 1;
                m_marker = ++m_cursor;
                yych = *m_cursor;
                basic_json_parser_41:
                if (yybm[0 + yych] & 128) {
                    goto basic_json_parser_40;
                }
                if (yych <= 'D') {
                    if (yych != '.') {
                        goto basic_json_parser_24;
                    }
                }
                else {
                    if (yych <= 'E') {
                        goto basic_json_parser_43;
                    }
                    if (yych == 'e') {
                        goto basic_json_parser_43;
                    }
                    goto basic_json_parser_24;
                }
                basic_json_parser_42:
                yych = *++m_cursor;
                if (yych <= '/') {
                    goto basic_json_parser_32;
                }
                if (yych <= '9') {
                    goto basic_json_parser_47;
                }
                goto basic_json_parser_32;
                basic_json_parser_43:
                yych = *++m_cursor;
                if (yych <= ',') {
                    if (yych != '+') {
                        goto basic_json_parser_32;
                    }
                }
                else {
                    if (yych <= '-') {
                        goto basic_json_parser_44;
                    }
                    if (yych <= '/') {
                        goto basic_json_parser_32;
                    }
                    if (yych <= '9') {
                        goto basic_json_parser_45;
                    }
                    goto basic_json_parser_32;
                }
                basic_json_parser_44:
                yych = *++m_cursor;
                if (yych <= '/') {
                    goto basic_json_parser_32;
                }
                if (yych >= ':') {
                    goto basic_json_parser_32;
                }
                basic_json_parser_45:
                ++m_cursor;
                yych = *m_cursor;
                if (yych <= '/') {
                    goto basic_json_parser_24;
                }
                if (yych <= '9') {
                    goto basic_json_parser_45;
                }
                goto basic_json_parser_24;
                basic_json_parser_47:
                yyaccept = 1;
                m_marker = ++m_cursor;
                yych = *m_cursor;
                if (yych <= 'D') {
                    if (yych <= '/') {
                        goto basic_json_parser_24;
                    }
                    if (yych <= '9') {
                        goto basic_json_parser_47;
                    }
                    goto basic_json_parser_24;
                }
                else {
                    if (yych <= 'E') {
                        goto basic_json_parser_43;
                    }
                    if (yych == 'e') {
                        goto basic_json_parser_43;
                    }
                    goto basic_json_parser_24;
                }
                basic_json_parser_49:
                yyaccept = 1;
                yych = *(m_marker = ++m_cursor);
                if (yych <= 'D') {
                    if (yych == '.') {
                        goto basic_json_parser_42;
                    }
                    goto basic_json_parser_24;
                }
                else {
                    if (yych <= 'E') {
                        goto basic_json_parser_43;
                    }
                    if (yych == 'e') {
                        goto basic_json_parser_43;
                    }
                    goto basic_json_parser_24;
                }
                basic_json_parser_50:
                yych = *++m_cursor;
                if (yych != 'l') {
                    goto basic_json_parser_32;
                }
                yych = *++m_cursor;
                if (yych != 's') {
                    goto basic_json_parser_32;
                }
                yych = *++m_cursor;
                if (yych != 'e') {
                    goto basic_json_parser_32;
                }
                ++m_cursor;
                {
                    return token_type::literal_false;
                }
                basic_json_parser_55:
                yych = *++m_cursor;
                if (yych != 'u') {
                    goto basic_json_parser_32;
                }
                yych = *++m_cursor;
                if (yych != 'e') {
                    goto basic_json_parser_32;
                }
                ++m_cursor;
                {
                    return token_type::literal_true;
                }
                basic_json_parser_59:
                yych = *++m_cursor;
                if (yych != 'l') {
                    goto basic_json_parser_32;
                }
                yych = *++m_cursor;
                if (yych != 'l') {
                    goto basic_json_parser_32;
                }
                ++m_cursor;
                {
                    return token_type::literal_null;
                }
            }

        }

        /// return string representation of last read token
        inline string_t get_token() const noexcept {
            return string_t(reinterpret_cast<typename string_t::const_pointer>(m_start),
                    static_cast<size_t>(m_cursor - m_start));
        }

        inline string_t get_string() const {
            string_t result;
            result.reserve(static_cast<size_t>(m_cursor - m_start - 2));

            // iterate the result between the quotes
            for (const lexer_char_t *i = m_start + 1; i < m_cursor - 1; ++i) {
                // process escaped characters
                if (*i == '\\') {
                    // read next character
                    ++i;

                    switch (*i) {
                        // the default escapes
                        case 't': {
                            result += "\t";
                            break;
                        }
                        case 'b': {
                            result += "\b";
                            break;
                        }
                        case 'f': {
                            result += "\f";
                            break;
                        }
                        case 'n': {
                            result += "\n";
                            break;
                        }
                        case 'r': {
                            result += "\r";
                            break;
                        }

                            // characters that are not "un"escsaped
                        case '\\': {
                            result += "\\\\";
                            break;
                        }
                        case '/': {
                            result += "\\/";
                            break;
                        }
                        case '"': {
                            result += "\\\"";
                            break;
                        }

                            // unicode
                        case 'u': {
                            // get code xxxx from uxxxx
                            auto codepoint = std::strtoul(std::string(reinterpret_cast<typename string_t::const_pointer>(i + 1),
                                    4).c_str(), nullptr, 16);

                            if (codepoint >= 0xD800 and codepoint <= 0xDBFF) {
                                // make sure there is a subsequent unicode
                                if ((i + 6 >= m_limit) or *(i + 5) != '\\' or *(i + 6) != 'u') {
                                    throw std::invalid_argument("missing low surrogate");
                                }

                                // get code yyyy from uxxxx\uyyyy
                                auto codepoint2 = std::strtoul(std::string(reinterpret_cast<typename string_t::const_pointer>
                                (i + 7), 4).c_str(), nullptr, 16);
                                result += to_unicode(codepoint, codepoint2);
                                // skip the next 11 characters (xxxx\uyyyy)
                                i += 11;
                            }
                            else {
                                // add unicode character(s)
                                result += to_unicode(codepoint);
                                // skip the next four characters (xxxx)
                                i += 4;
                            }
                            break;
                        }
                    }
                }
                else {
                    // all other characters are just copied to the end of the
                    // string
                    result.append(1, static_cast<typename string_t::value_type>(*i));
                }
            }

            return result;
        }

        inline number_float_t get_number() const {
            // conversion
            typename string_t::value_type *endptr;
            const auto float_val = std::strtod(reinterpret_cast<typename string_t::const_pointer>(m_start),
                    &endptr);

            // return float_val if the whole number was translated and NAN
            // otherwise
            return (reinterpret_cast<lexer_char_t *>(endptr) == m_cursor) ? float_val : NAN;
        }

    private:
        /// the buffer
        const lexer_char_t *m_content = nullptr;
        /// pointer to he beginning of the current symbol
        const lexer_char_t *m_start = nullptr;
        /// pointer to the current symbol
        const lexer_char_t *m_cursor = nullptr;
        /// pointer to the end of the buffer
        const lexer_char_t *m_limit = nullptr;
    };

    class json_decoder{
    public:

        /// constructor for strings
        inline json_decoder(const string_t &s) : m_buffer(s), m_lexer(m_buffer) {
            // read first token
            get_token();
        }

        /// a parser reading from an input stream
        inline json_decoder(std::istream &_is, std::ostream &ostream) {
            while (_is) {
                string_t input_line;
                std::getline(_is, input_line);
                m_buffer += input_line;
            }

            // initializer lexer
            m_lexer = lexer(m_buffer);

            // read first token
            get_token();
        }

        /// public parser interface
        inline void parse(pson & value) {
            parse_internal(value);
            expect(lexer::token_type::end_of_input);
        }

    private:

        /// the actual parser
        void parse_internal(pson & value){
            switch (last_token) {
                case (lexer::token_type::begin_object): {
                    // explicitly set result to object to cope with {}
                    // initialize object
                    pson_object & object = ((pson_object &)value);

                    // read next token
                    get_token();

                    // closing } -> we are done
                    if (last_token == lexer::token_type::end_object) {
                        get_token();
                        return;
                    }

                    // otherwise: parse key-value pairs
                    do {
                        // ugly, but could be fixed with loop reorganization
                        if (last_token == lexer::token_type::value_separator) {
                            get_token();
                        }

                        // store key
                        expect(lexer::token_type::value_string);
                        const auto key = m_lexer.get_string();

                        // parse separator (:)
                        get_token();
                        expect(lexer::token_type::name_separator);

                        // parse value
                        get_token();
                        parse_internal(object[key.c_str()]);
                    }
                    while (last_token == lexer::token_type::value_separator);

                    // closing }
                    expect(lexer::token_type::end_object);
                    get_token();
                    return;
                }

                case (lexer::token_type::begin_array): {
                    // explicitly set result to object to cope with []
                    pson_array & array = ((pson_array &)value);

                    // read next token
                    get_token();

                    // closing ] -> we are done
                    if (last_token == lexer::token_type::end_array) {
                        get_token();
                        return;
                    }

                    // otherwise: parse values
                    do {
                        // ugly, but could be fixed with loop reorganization
                        if (last_token == lexer::token_type::value_separator) {
                            get_token();
                        }

                        // parse value
                        parse_internal(array.create_item());
                    }
                    while (last_token == lexer::token_type::value_separator);

                    // closing ]
                    expect(lexer::token_type::end_array);
                    get_token();

                    return;
                }

                case (lexer::token_type::literal_null): {
                    value.set_null();
                    get_token();
                    return;
                }

                case (lexer::token_type::value_string):
                    value = m_lexer.get_string().c_str();
                    get_token();
                    break;

                case (lexer::token_type::literal_true):
                    get_token();
                    value = true;
                    break;

                case (lexer::token_type::literal_false):
                    get_token();
                    value = false;
                    break;

                case (lexer::token_type::value_number): {
                    auto float_val = m_lexer.get_number();

                    // NAN is returned if token could not be translated
                    // completely
                    if (std::isnan(float_val)) {
                        throw std::invalid_argument(std::string("parse error - ") +
                                m_lexer.get_token() + " is not a number");
                    }

                    get_token();
                    value = float_val;
                }
                break;

                default: {
                    std::string error_msg = "parse error - unexpected \'";
                    error_msg += m_lexer.get_token();
                    error_msg += "\' (";
                    error_msg += lexer::token_type_name(last_token) + ")";
                    throw std::invalid_argument(error_msg);
                }
            }
        }

        /// get next token from lexer
        inline typename lexer::token_type get_token() {
            last_token = m_lexer.scan();
            return last_token;
        }

        inline void expect(typename lexer::token_type t) const {
            if (t != last_token) {
                std::string error_msg = "parse error - unexpected \'";
                error_msg += m_lexer.get_token();
                error_msg += "\' (" + lexer::token_type_name(last_token);
                error_msg += "); expected " + lexer::token_type_name(t);
                throw std::invalid_argument(error_msg);
            }
        }

    private:
        /// the buffer
        string_t m_buffer;
        /// the type of the last read token
        typename lexer::token_type last_token = lexer::token_type::uninitialized;
        /// the lexer
        lexer m_lexer;
    };

}

#endif