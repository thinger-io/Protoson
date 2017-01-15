#include <iostream>
#include <fstream>
#include "../src/pson.h"
#include "../src/util/json_encoder.hpp"
#include "../src/util/json_decoder.hpp"
#include "../src/util/pson_json_transcoder.hpp"

protoson::dynamic_memory_allocator alloc;
protoson::memory_allocator&protoson::pool = alloc;

using namespace std;

class memory_reader : public protoson::pson_decoder {
private:
    char* buffer_;
    size_t size_;
public:
    memory_reader(char *buffer, size_t size) : buffer_(buffer), size_(size){
    }

protected:
    virtual bool read(void *buffer, size_t size) {
        if(read_+size<size_){
            memcpy(buffer, &buffer_[read_], size);
            return pson_decoder::read(buffer, size);
        }else{
            return false;
        }
    }
};

int main() {
    // example for testing decoding random binary data (like corrupt input).
    for(int i=0; i<100000000; i++){
        char memory_buffer[2048];
        for(int j=0;j<2048;j++){
            memory_buffer[j] = rand() % 255;
        }
        memory_reader pson_reader(memory_buffer, 2048);
        protoson::pson object;
        if(pson_reader.decode(object)){
            std::ostringstream json_result;
            protoson::json_encoder json_encoder(json_result);
            json_encoder.encode(object);
            //std::cout << "Object decoded!!" << std::endl;
            //std::cout << "\t" << json_result.str() << std::endl;
        }
    }

}