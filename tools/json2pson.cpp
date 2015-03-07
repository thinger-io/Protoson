#include <iostream>
#include "../src/pson_encoder.hpp"
#include "../src/util/json_decoder.hpp"

using namespace std;

class cout_writter : public protoson::pson_encoder {
protected:
    virtual void write(const void *buffer, size_t size) {
        cout.write((char*)buffer, size);
    }
};

int main(int argc, char **argv) {
    string json;

    string lineInput;
    while (getline(cin,lineInput)) {
        json += lineInput;
    }

    protoson::pson value;
    protoson::json_decoder decoder(json);
    decoder.parse(value);

    cout_writter writter;
    writter.encode(value);

    return 0;
}