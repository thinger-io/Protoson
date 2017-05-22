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
#include "../src/pson.h"
#include "../src/util/json_decoder.hpp"

using namespace std;
using namespace protoson;

dynamic_memory_allocator alloc;
memory_allocator&protoson::pool = alloc;

class cout_writter : public pson_encoder {
protected:
    virtual bool write(const void *buffer, size_t size) {
        cout.write((char*)buffer, size);
        return true;
    }
};

int main(int argc, char **argv) {

    string json;

    // read input from cin
    if(argc==1){
        string lineInput;
        while (getline(cin,lineInput)) {
            json += lineInput;
        }
    }else{
        std::ifstream t(argv[1]);
        std::stringstream buffer;
        buffer << t.rdbuf();
        json = buffer.str();
    }

    // parse and decode json
    nlohmann::json jsonValue;
    try{
        jsonValue = nlohmann::json::parse(json);
    }catch(std::invalid_argument e){
        return -1;
    }

    // convert json to pson
    pson value;
    nlohmann::to_pson(jsonValue, value);

    //std::cout << std::setw(4) << jsonValue << std::endl;

    // encode pson to binary
    cout_writter writter;
    writter.encode(value);

    return 0;
}