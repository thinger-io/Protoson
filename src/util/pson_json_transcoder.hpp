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

#ifndef PSON_JSON_TRANSCODER_HPP
#define PSON_JSON_TRANSCODER_HPP

#include <stdint.h>
#include <stddef.h>
#include <sstream>
#include "../pson.h"

namespace protoson {

    class pson_json_transcoder : public pson_decoder {

    private:
        std::ostream& stream_;

    public:
        pson_json_transcoder(std::ostream& stream) : stream_(stream){
        }

    protected:
        template<class T>
        void encode(T value){
            stream_ << value;
        }

    public:

        bool transcode_object(size_t size){
            encode('{');
            size_t start_read = bytes_read();
            while(size-(bytes_read()-start_read)>0){
                if(!transcode_pair()){
                    return false;
                }
                if(size-(bytes_read()-start_read)>0){
                    encode(',');
                }
            }
            encode('}');
            return true;
        }

        bool transcode_array(size_t size){
            encode('[');
            size_t start_read = bytes_read();
            while(size-(bytes_read()-start_read)>0){
                if(!transcode_value()){
                    return false;
                }
                if(size-(bytes_read()-start_read)>0){
                    encode(',');
                }
            }
            encode(']');
            return true;
        }

        bool transcode_string(size_t size){
            encode('"');
            char byte;
            for(size_t i=0; i<size; i++){
                if(!read(&byte, 1)) return false;
                encode(byte);
            }
            encode('"');
            return true;
        }

        bool transcode_pair(){
            uint32_t name_size = 0;
            if(!pb_decode_varint32(name_size)) return false;
            if(!transcode_string(name_size)) return false;
            encode(":");
            if(!transcode_value()) return false;
            return true;
        }

        bool transcode_value() {
            uint32_t field_number;
            pb_wire_type wire_type;
            pb_decode_tag(wire_type, field_number);
            if(wire_type==pb_wire_type::length_delimited){
                uint32_t size = 0;
                if(!pb_decode_varint32(size)) return false;
                switch(field_number) {
                    case pson::string_field:
                        return transcode_string(size);
                    case pson::object_field:
                        return transcode_object(size);
                    case pson::array_field:
                        return transcode_array(size);
                }
            }else {
                switch (field_number) {
                    case pson::true_field:
                        encode("true");
                        break;
                    case pson::false_field:
                        encode("false");
                        break;
                    case pson::one_field:
                        encode("1");
                        break;
                    case pson::zero_field:
                        encode("0");
                        break;
                    case pson::svarint_field:
                    {
                        uint64_t varint = 0;
                        if(!pb_decode_varint64(varint)) return false;
                        encode(-varint);
                    }
                        break;
                    case pson::varint_field:
                    {
                        uint64_t varint = 0;
                        if(!pb_decode_varint64(varint)) return false;
                        encode(varint);
                    }
                        break;
                    case pson::float_field: {
                        float value;
                        if(!read(&value, 4)) return false;
                        encode(value);
                    }
                        break;
                    case pson::double_field: {
                        double value;
                        if(!read(&value, 8)) return false;
                        encode(value);
                    }
                    case pson::empty_bytes:
                    case pson::empty_string:
                        encode("\"\"");
                        break;
                    case pson::null_field:
                        encode("null");
                        break;
                }
            }
            return true;
        }
    };
}

#endif