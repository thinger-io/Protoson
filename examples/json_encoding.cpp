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

#include <iostream>
#include "../src/pson.h"
#include "../src/util/json_encoder.hpp"

using namespace protoson;
using namespace std;

// set dynamic memory allocator
dynamic_memory_allocator alloc;
memory_allocator&protoson::pool = alloc;

int main() {

    pson object;

    // adding some basic types
    object["hello"] = "world!";
    object["time"] = 1234567890;
    object["float"] = 0.01234;
    object["boolean"] = true;
    object["otherbool"] = "false";
    object["null"];
    // adding object in array
    pson_object & array_object = object["obj"];
    array_object["what"] = "that";

    // adding array
    pson_array & array = object["arr"];

    // adding values in array
    array.add(1);
    array.add(2);
    array.add(3);


    // adding nested empty array
    pson_array & nested_array = object["nested"]["array"];

    // adding nested empty object
    pson_object& nested_object = object["nested"]["object"];

    // encode to std::cout (any std::ostream should be valid)
    json_encoder encoder(cout);
    cout << "[*] Encoding to Json..." << endl << "\t";
    encoder.encode(object);
    cout << endl;

    return 0;
}