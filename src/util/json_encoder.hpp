// The MIT License (MIT)
//
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

#ifndef JSON_ENCODER_HPP
#define JSON_ENCODER_HPP

#include <sstream>
#include <numeric>
#include <cmath>
#include "../pson.h"

namespace protoson {
    class json_encoder{

    public:
        json_encoder(std::ostream& stream) : stream_(stream){
        }

    private:
        std::ostream& stream_;

    public:

        static std::size_t extra_space(const std::string& s) noexcept
        {
            return std::accumulate(s.begin(), s.end(), size_t{},
               [](size_t res, typename std::string::value_type c)
               {
                   switch (c)
                   {
                       case '"':
                       case '\\':
                       case '\b':
                       case '\f':
                       case '\n':
                       case '\r':
                       case '\t':
                       {
                           // from c (1 byte) to \x (2 bytes)
                           return res + 1;
                       }

                       default:
                       {
                           if (c >= 0x00 and c <= 0x1f)
                           {
                               // from c (1 byte) to \uxxxx (6 bytes)
                               return res + 5;
                           }

                           return res;
                       }
                   }
               });
        }

        static std::string escape_string(const std::string& s)
        {
            const auto space = extra_space(s);
            if (space == 0)
            {
                return s;
            }

            // create a result string of necessary size
            std::string result(s.size() + space, '\\');
            std::size_t pos = 0;

            for (const auto& c : s)
            {
                switch (c)
                {
                    // quotation mark (0x22)
                    case '"':
                    {
                        result[pos + 1] = '"';
                        pos += 2;
                        break;
                    }

                        // reverse solidus (0x5c)
                    case '\\':
                    {
                        // nothing to change
                        pos += 2;
                        break;
                    }

                        // backspace (0x08)
                    case '\b':
                    {
                        result[pos + 1] = 'b';
                        pos += 2;
                        break;
                    }

                        // formfeed (0x0c)
                    case '\f':
                    {
                        result[pos + 1] = 'f';
                        pos += 2;
                        break;
                    }

                        // newline (0x0a)
                    case '\n':
                    {
                        result[pos + 1] = 'n';
                        pos += 2;
                        break;
                    }

                        // carriage return (0x0d)
                    case '\r':
                    {
                        result[pos + 1] = 'r';
                        pos += 2;
                        break;
                    }

                        // horizontal tab (0x09)
                    case '\t':
                    {
                        result[pos + 1] = 't';
                        pos += 2;
                        break;
                    }

                    default:
                    {
                        if (c >= 0x00 and c <= 0x1f)
                        {
                            // convert a number 0..15 to its hex representation
                            // (0..f)
                            static const char hexify[16] =
                                    {
                                            '0', '1', '2', '3', '4', '5', '6', '7',
                                            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
                                    };

                            // print character c as \uxxxx
                            for (const char m :
                                    { 'u', '0', '0', hexify[c >> 4], hexify[c & 0x0f]
                                    })
                            {
                                result[++pos] = m;
                            }

                            ++pos;
                        }
                        else
                        {
                            // all other characters are added as-is
                            result[pos++] = c;
                        }
                        break;
                    }
                }
            }

            return result;
        }

        template<class T>
        void encode(T value, bool quoted = false){
            if(quoted){
                stream_ << '"' << value << '"';
            }else{
                stream_ << value;
            }
        }

        void encode(const char* value, bool quoted = false){
            if(quoted){
                stream_ << '"' << escape_string(value) << '"';
            }else{
                stream_ << value;
            }
        }

        void encode(pson_object & object){
            encode('{');
            pson_container<pson_pair>::iterator it = object.begin();
            while(it.valid()){
                encode(it.item());
                it.next();
                if(it.valid()){
                    encode(',');
                }
            }
            encode('}');
        }

        void encode(pson_array & array){
            encode('[');
            pson_container<pson>::iterator it = array.begin();
            while(it.valid()){
                encode(it.item());
                it.next();
                if(it.valid()){
                    encode(',');
                }
            }
            encode(']');
        }

        void encode(pson_pair & pair){
            if(pair.name()!=NULL){
                encode(pair.name(), true);
            }else{
                encode("");
            }
            encode(':');
            encode(pair.value());
        }

        void encode(pson & value, bool root=false)
        {
            switch(value.get_type())
            {
                case pson::zero_field:
                    encode('0');
                    break;
                case pson::one_field:
                    encode('1');
                    break;
                case pson::true_field:
                    encode("true");
                    break;
                case pson::false_field:
                    encode("false");
                    break;
                case pson::svarint_field:
                    encode((int64_t) value);
                    break;
                case pson::varint_field:
                    encode((uint64_t)value);
                    break;
                case pson::float_field:
                {
                    float val = (float) value;
                    if(std::isnan(val)){
                        encode("null");
                    }else{
                        encode(val);
                    }
                }
                    break;
                case pson::double_field:
                {
                    float val = (double) value;
                    if(std::isnan(val)){
                        encode("null");
                    }else{
                        encode(val);
                    }
                }
                    break;
                case pson::string_field:
                    encode((const char*)value, !root);
                    break;
                case pson::empty_string:
                case pson::empty_bytes:
                    encode("", !root);
                    break;
                case pson::object_field:
                    encode((pson_object &)value);
                    break;
                case pson::array_field:
                    encode((pson_array &)value);
                    break;
                case pson::bytes_field:
                    if(!root){ // binary fields are not supported inside a JSON tree
                        encode("", true);
                    }else{
                        uint8_t * data = NULL;
                        size_t size = 0;
                        value.get_bytes(data, size);
                        stream_.write((const char*)data, size);
                    }
                    break;
                case pson::null_field:
                    encode("null");
                    break;
                case pson::empty:
                    encode("{}");
                    break;
            }
        }
    };
}

#endif