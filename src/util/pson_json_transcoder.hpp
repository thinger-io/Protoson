// The MIT License (MIT)
//
// Copyright (c) 2015 THINGER LTD
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

        void transcode_object(size_t size){
            encode('{');
            size_t start_read = bytes_read();
            while(size-(bytes_read()-start_read)>0){
                transcode_pair();
                if(size-(bytes_read()-start_read)>0){
                    encode(',');
                }
            }
            encode('}');
        }

        void transcode_array(size_t size){
            encode('[');
            size_t start_read = bytes_read();
            while(size-(bytes_read()-start_read)>0){
                transcode_value();
                if(size-(bytes_read()-start_read)>0){
                    encode(',');
                }
            }
            encode(']');
        }

        void transcode_string(size_t size){
            encode('"');
            char byte;
            for(size_t i=0; i<size; i++){
                read(&byte, 1);
                encode(byte);
            }
            encode('"');
        }

        void transcode_pair(){
            uint32_t name_size = pb_decode_varint32();
            transcode_string(name_size);
            encode(":");
            transcode_value();
        }

        void transcode_value() {
            uint32_t field_number;
            pb_wire_type wire_type;
            pb_decode_tag(wire_type, field_number);
            if(wire_type==pb_wire_type::length_delimited){
                uint32_t size = pb_decode_varint32();
                switch(field_number) {
                    case pson::string_field:
                        transcode_string(size);
                        break;
                    case pson::object_field:
                        transcode_object(size);
                        break;
                    case pson::array_field:
                        transcode_array(size);
                        break;
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
                        encode(-(int64_t)pb_decode_varint64());
                        break;
                    case pson::varint_field:
                        encode(pb_decode_varint64());
                        break;
                    case pson::float_field: {
                        float value;
                        read(&value, 4);
                        encode(value);
                    }
                        break;
                    case pson::double_field: {
                        double value;
                        read(&value, 8);
                        encode(value);
                    }
                        break;
                }
            }
        }
    };
}

#endif