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
#include "../src/util/json_decoder.hpp"

using namespace protoson;
using namespace std;

// set dynamic memory allocator
dynamic_memory_allocator alloc;
memory_allocator&protoson::pool = alloc;

int main() {
    pson decoded_object;
    json_decoder decoder("{\"int\":255,\"float\":222.5,\"double\":220.222,\"bool\":false,\"string\":\"hello!\",\"null\":null,\"array\":[223,false,\"world!\",{\"a\":true,\"b\":false,\"c\":false},[false,true,false]],\"nested\":{\"int\":555,\"bool\":false,\"double\":3.14,\"array\":[],\"object\":{}}}");
    cout << "[*] Decoding  Json..." << endl;
    decoder.parse(decoded_object);

    cout << "[*] Testing decoded json..." << endl;
    cout << "[\"int\"] = " << (int) decoded_object["int"] << endl;
    cout << "[\"float\"] = " << (float) decoded_object["float"] << endl;
    cout << "[\"double\"] = " << (double) decoded_object["double"] << endl;
    cout << "[\"bool\"] = " << (bool) decoded_object["bool"] << endl;
    cout << "[\"string\"] = " << (const char*) decoded_object["string"] << endl;
    cout << "[\"null\"] = " << decoded_object["null"].is_null() << endl;

    return 0;
}