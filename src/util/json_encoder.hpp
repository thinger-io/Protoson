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
#include "../pson.h"

namespace protoson {
    class json_encoder{

    public:
        json_encoder(std::ostream& stream) : stream_(stream){
        }

    private:
        std::ostream& stream_;

    public:

        template<class T>
        void encode(T value, bool quoted = false){
            if(quoted){
                stream_ << '"' << value << '"';
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
            encode(pair.name(), true);
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
                    encode((float)value);
                    break;
                case pson::double_field:
                    encode((double)value);
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
                        const void * data = NULL;
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