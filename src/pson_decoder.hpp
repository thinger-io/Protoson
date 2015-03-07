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

#ifndef PROTOSON_DECODER_HPP
#define PROTOSON_DECODER_HPP

#include "pson.h"

namespace protoson {
    class pson_decoder {

    protected:
        size_t read_;

        virtual bool read(void* buffer, size_t size){
            read_+=size;
            return true;
        }

    public:

        pson_decoder() : read_(0) {

        }

        size_t bytes_read(){
            return read_;
        }

        bool pb_decode_tag(pb_wire_type& wire_type, uint32_t& field_number)
        {
            uint32_t temp = pb_decode_varint32();
            wire_type = (pb_wire_type)(temp & 0x07);
            field_number = temp >> 3;
            return true;
        }

        uint32_t pb_decode_varint32()
        {
            uint32_t varint = 0;
            uint8_t byte;
            uint8_t bit_pos = 0;
            do{
                if(!read(&byte, 1) || bit_pos>=32){
                    return varint;
                }
                varint |= (uint32_t)(byte&0x7F) << bit_pos;
                bit_pos += 7;
            }while(byte>0x80);
            return varint;
        }

        uint64_t pb_decode_varint64()
        {
            uint64_t varint = 0;
            uint8_t byte;
            uint8_t bit_pos = 0;
            do{
                if(!read(&byte, 1) || bit_pos>=64){
                    return varint;
                }
                varint |= (uint32_t)(byte&0x7F) << bit_pos;
                bit_pos += 7;
            }while(byte>0x80);
            return varint;
        }

        bool pb_skip(size_t size){
            uint8_t byte;
            bool success = true;
            for(size_t i=0; i<size; i++){
                success &= read(&byte, 1);
            }
            return success;
        }

        bool pb_skip_varint(){
            uint8_t byte;
            bool success = true;
            do{
                success &= read(&byte, 1);
            }while(byte>0x80);
            return success;
        }

        bool pb_read_string(char *str, size_t size){
            bool success = read(str, size);
            str[size]=0;
            return success;
        }

        bool pb_read_varint(pson& value)
        {
            char temp[10];
            uint8_t byte=0;
            uint8_t bytes_read=0;
            do{
                if(!read(&byte, 1)) return false;
                temp[bytes_read] = byte;
                bytes_read++;
            }while(byte>=0x80);
            memcpy(value.allocate(bytes_read), temp, bytes_read);
            return true;
        }

    public:

        void decode(pson_object & object, size_t size){
            size_t start_read = bytes_read();
            while(size-(bytes_read()-start_read)>0){
                decode(object.new_item());
            }
        }

        void decode(pson_array & array, size_t size){
            size_t start_read = bytes_read();
            while(size-(bytes_read()-start_read)>0){
                decode(array.new_item());
            }
        }

        void decode(pson_pair & pair){
            uint32_t name_size = pb_decode_varint32();
            pb_read_string(pair.allocate_name(name_size+1), name_size);
            decode(pair.value());
        }

        void decode(pson& value) {
            uint32_t field_number;
            pb_wire_type wire_type;
            pb_decode_tag(wire_type, field_number);
            value.set_type((pson::field_type)field_number);
            if(wire_type==pb_wire_type::length_delimited){
                uint32_t size = pb_decode_varint32();
                switch(field_number){
                    case pson::string_field:
                        pb_read_string((char*)value.allocate(size + 1), size);
                        break;
                    case pson::object_field:
                        value.set_value(new (pool) pson_object);
                        decode(*(pson_object *)value.get_value(), size);
                        break;
                    case pson::array_field:
                        value.set_value(new (pool) pson_array);
                        decode(*(pson_array *)value.get_value(), size);
                        break;
                    default:
                        pb_skip(size);
                        break;
                }
            }else {
                switch (field_number) {
                    case pson::svarint_field:
                    case pson::varint_field:
                        pb_read_varint(value);
                        break;
                    case pson::float_field:
                        read(value.allocate(4), 4);
                        break;
                    case pson::double_field:
                        read(value.allocate(8), 8);
                        break;
                    default:
                        break;
                }
            }
        }
    };
}

#endif