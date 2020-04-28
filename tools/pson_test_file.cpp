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
#include <fstream>
#include "../src/util/json_encoder.hpp"

using namespace std;
using namespace protoson;

dynamic_memory_allocator alloc;
memory_allocator& protoson::pool = alloc;

class pson_file_reader : public pson_decoder {
public:
    pson_file_reader(const std::string file) : file_(file){
    }
protected:
    virtual bool read(void *buffer, size_t size) {
        file_.read((char *) buffer, size);
        if(file_.fail()){
            return false;
        }else{
            return pson_decoder::read(buffer, size);
        }
    }
private:
    std::ifstream file_;
};

int main(int argc, char **argv) {
    pson value;
    std::cout << argv[1] << std::endl;
    pson_file_reader reader(argv[1]);
    if(reader.decode(value)){
        std::cout <<"Decoding ok!" << std::endl;
    }else{
        std::cerr << "Decoding error!" << std::endl;
    }
    //json_encoder encoder(cout);
    //encoder.encode(value);
    return 0;
}