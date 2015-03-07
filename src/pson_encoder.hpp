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

#ifndef PROTOSON_ENCODER_HPP
#define PROTOSON_ENCODER_HPP

#include "pson.h"

namespace protoson {
    class pson_encoder {

    protected:
        size_t written_;

        virtual void write(const void* buffer, size_t size){
            written_+=size;
        }

    public:

        pson_encoder() : written_(0) {
        }

        size_t bytes_written(){
            return written_;
        }

        void pb_encode_tag(pb_wire_type wire_type, uint32_t field_number){
            uint64_t tag = ((uint64_t)field_number << 3) | wire_type;
            pb_encode_varint(tag);
        }

        void pb_encode_varint(uint32_t field, uint64_t value)
        {
            pb_encode_tag(varint, field);
            pb_encode_varint(value);
        }

        void pb_write_varint(void * buffer)
        {
            uint8_t byte=0;
            size_t bytes_written=0;
            do{
                byte = *((uint8_t*)buffer + bytes_written);
                write(&byte, 1);
                bytes_written++;
            }while(byte>=0x80);
        }

        void pb_encode_varint(uint64_t value)
        {
            do
            {
                uint8_t byte = (uint8_t)(value & 0x7F);
                value >>= 7;
                if(value>0) byte |= 0x80;
                write(&byte, 1);
            }while(value>0);
        }

        void pb_encode_string(const char* str, uint32_t field_number){
            pb_encode_tag(length_delimited, field_number);
            pb_encode_string(str);
        }

        void pb_encode_string(const char* str){
            size_t string_size = strlen(str);
            pb_encode_varint(string_size);
            write(str, string_size);
        }

        template<class T>
        void pb_encode_submessage(T& element, uint32_t field_number)
        {
            pb_encode_tag(length_delimited, field_number);
            pson_encoder sink;
            sink.encode(element);
            pb_encode_varint(sink.bytes_written());
            encode(element);
        }

        void pb_encode_fixed32(void* value){
            write(value, 4);
        }

        void pb_encode_fixed64(void* value){
            write(value, 8);
        }

        void pb_encode_fixed32(uint32_t field, void*value)
        {
            pb_encode_tag(fixed_32, field);
            pb_encode_fixed32(value);
        }

        void pb_encode_fixed64(uint32_t field, void*value)
        {
            pb_encode_tag(fixed_64, field);
            pb_encode_fixed64(value);
        }

    public:

        void encode(pson_object & object){
            pson_container<pson_pair>::iterator it = object.begin();
            while(it.valid()){
                encode(it.item());
                it.next();
            }
        }

        void encode(pson_array & array){
            pson_container<pson>::iterator it = array.begin();
            while(it.valid()){
                encode(it.item());
                it.next();
            }
        }

        void encode(pson_pair & pair){
            pb_encode_string(pair.name());
            encode(pair.value());
        }

        void encode(pson & value) {
            switch (value.get_type()) {
                case pson::true_field:
                case pson::false_field:
                case pson::one_field:
                case pson::zero_field:
                    pb_encode_tag(varint, value.get_type());
                    break;
                case pson::string_field:
                    pb_encode_string((const char*)value.get_value(), pson::string_field);
                    break;
                case pson::svarint_field:
                case pson::varint_field:
                    pb_encode_tag(varint, value.get_type());
                    pb_write_varint(value.get_value());
                    break;
                case pson::float_field:
                    pb_encode_fixed32(pson::float_field, value.get_value());
                    break;
                case pson::double_field:
                    pb_encode_fixed64(pson::double_field, value.get_value());
                    break;
                case pson::object_field:
                    pb_encode_submessage(*(pson_object *) value.get_value(), pson::object_field);
                    break;
                case pson::array_field:
                    pb_encode_submessage(*(pson_array *) value.get_value(), pson::array_field);
                    break;
                default:
                    pb_encode_tag(varint, pson::null_field);
                    break;
            }
        }
    };
}

#endif