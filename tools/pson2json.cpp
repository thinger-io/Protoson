#include <iostream>
#include "../src/pson_decoder.hpp"
#include "../src/util/json_encoder.hpp"

protoson::dynamic_memory_allocator alloc;
protoson::memory_allocator&protoson::pool = alloc;

using namespace std;

class pson_reader : public protoson::pson_decoder {
protected:
    virtual bool read(void *buffer, size_t size) {
        cin.read((char*)buffer, size);
        return pson_decoder::read(buffer, size);
    }
};

class json_writer : public protoson::json_encoder{
public:
    json_writer() : protoson::json_encoder(cout){
    }
};

int main(int argc, char **argv) {
    protoson::pson value;
    pson_reader reader;
    reader.decode(value);
    json_writer writer;
    writer.encode(value);
    return 0;
}