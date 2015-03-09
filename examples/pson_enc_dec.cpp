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

#include <iostream>
#include "../src/pson.h"

using namespace protoson;
using namespace std;

// set dynamic memory allocator
dynamic_memory_allocator alloc;
memory_allocator&protoson::pool = alloc;

// helper class for encoding to memory (you can create your own encoders writing to file, socket, etc)
class memory_writer : public pson_encoder {
private:
    char* buffer_;
public:
    memory_writer(char *buffer) : buffer_(buffer){
    }

protected:
    virtual void write(const void *buffer, size_t size) {
        memcpy(&buffer_[written_], buffer, size);
        pson_encoder::write(buffer, size);
    }
};

// helper class for decoding form memory
class memory_reader : public protoson::pson_decoder {
private:
    char* buffer_;
public:
    memory_reader(char *buffer) : buffer_(buffer){
    }

protected:
    virtual bool read(void *buffer, size_t size) {
        memcpy(buffer, &buffer_[read_], size);
        return pson_decoder::read(buffer, size);
    }
};

int main() {

    pson object;

    // adding some basic types
    object["int"] = 255;
    object["float"] = 222.5f;
    object["double"] = 220.222;
    object["bool"] = false;
    object["string"] = "hello!";
    object["null"];

    // adding array
    pson_array & array = object["array"];

    // adding values in array
    array.add(223);
    array.add(false);
    array.add("world!");

    // adding object in array
    pson_object & array_object = array.create_item();
    array_object["a"] = true;
    array_object["b"] = false;
    array_object["c"] = false;

    // adding array in array
    pson_array & sub_array = array.create_item();
    sub_array.add(false);
    sub_array.add(true);
    sub_array.add(false);

    // adding nested items
    object["nested"]["int"] = 555;
    object["nested"]["bool"] = false;
    object["nested"]["double"] = 3.14;

    // adding nested empty array
    pson_array & nested_array = object["nested"]["array"];

    // adding nested empty object
    pson_object& nested_object = object["nested"]["object"];

    // reserve memory for encoding
    char memory_buffer[2048];

    // encode
    memory_writer pson_writer(memory_buffer);
    cout << "[*] Encoding to Protoson..." << endl;
    pson_writer.encode(object);
    cout << "[*] Encoding Size: " <<  pson_writer.bytes_written() << " bytes" << endl;

    // decode
    memory_reader pson_reader(memory_buffer);
    protoson::pson decoded_object;
    cout << "[*] Decoding from Protoson..." << endl;
    pson_reader.decode(decoded_object);

    // display some decoded elements
    cout << "[*] Testing decoded Pson..." << endl;
    cout << "[\"int\"] = " << (int) decoded_object["int"] << endl;
    cout << "[\"float\"] = " << (float) decoded_object["float"] << endl;
    cout << "[\"double\"] = " << (double) decoded_object["double"] << endl;
    cout << "[\"bool\"] = " << (bool) decoded_object["bool"] << endl;
    cout << "[\"string\"] = " << (const char*) decoded_object["string"] << endl;
    cout << "[\"null\"] = " << decoded_object["null"].is_null() << endl;



    return 0;
}