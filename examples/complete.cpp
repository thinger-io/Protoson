#include <iostream>
#include <fstream>
#include <chrono>
#include <stddef.h>
#include <iomanip>
#include "../src/pson.h"
#include "../src/util/json_encoder.hpp"
#include "../src/util/json_decoder.hpp"
#include "../src/util/pson_json_transcoder.hpp"

protoson::dynamic_memory_allocator alloc;
protoson::memory_allocator&protoson::pool = alloc;

using namespace std;

class memory_writer : public protoson::pson_encoder {
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

class json_transcoder : public protoson::pson_json_transcoder{
private:
    char* buffer_;
public:
    json_transcoder(char *buffer) : buffer_(buffer){
    }

protected:
    virtual bool read(void *buffer, size_t size) {
        memcpy(buffer, &buffer_[read_], size);
        return pson_decoder::read(buffer, size);
    }
};

template<typename TimeT = std::chrono::microseconds>
struct measure
{
    template<typename F, typename ...Args>
    static typename TimeT::rep execution(F func, Args&&... args)
    {
        auto start = std::chrono::system_clock::now();
        func(std::forward<Args>(args)...);
        auto duration = std::chrono::duration_cast< TimeT>
                (std::chrono::system_clock::now() - start);
        return duration.count();
    }
};

int main() {
    char memory_buffer[2048];
    memory_writer pson_writer(memory_buffer);
    memory_reader pson_reader(memory_buffer);
    json_transcoder json_transcoder(memory_buffer);

    protoson::pson object;

    object["float"] = 33.25f;
    object["double_zero"] = 0.0;
    object["double_one"] = 1.0;
    object["int"] = 333;
    object["sint"] = -554456;
    object["string"] = ":=)";
    object["bool_true"] = true;
    object["bool_false"] = false;
    object["one"] = 1;
    object["zero"] = 0;
    object["nested"]["float"] = 33333.00;
    object["nested"]["double"] = 45.1234321;
    object["nested"]["bool"] = false;
    object["more"] = 25648;

    protoson::pson_object & nested = object["nested"];
    nested["another_nested"] = 34;

    protoson::pson_array & array = object["nested"]["array"];
    array.add(223);
    array.add(false);
    array.add(true);
    protoson::pson_object & array_object = array.create_item();
    array_object["a"] = true;
    array_object["b"] = false;
    array_object["c"] = false;
    protoson::pson_array & sub_array(array.create_item());
    sub_array.add(false);
    sub_array.add(true);
    sub_array.add(false);
    sub_array.add(true);
    array.add(1);
    array.add(0);
    array.add("hello");
    array.add(224.6569874);

    // TEST JSON
    std::ostringstream json_result;
    protoson::json_encoder json_encoder(json_result);
    json_encoder.encode(object);
    std::string parsed_json = json_result.str();

    std::cout << "[*] Json Encoding is (" <<  parsed_json.size() << " bytes):" << std::endl;
    std::cout << "\t" << json_result.str() << std::endl;

    std::cout << "[*] Encoding Time: " << measure<>::execution([&]{pson_writer.encode(object);}) << " μs" << std::endl;
    std::cout << "[*] Encoding Size: " <<  pson_writer.bytes_written() << " bytes" << std::endl;
    std::cout << "[*] Protoson is (" << pson_writer.bytes_written() << " bytes): " << (pson_writer.bytes_written()/(float)parsed_json.size())*100.0 << "%" << std::endl;

    protoson::pson decoded_object;
    std::cout << "[*] Decoding from Protoson..." << std::endl;
    std::cout << "[*] Decoding Time: " << measure<>::execution([&]{pson_reader.decode(decoded_object);}) << " μs" << std::endl;

    json_result.str(std::string());
    json_encoder.encode(decoded_object);
    std::cout << "\t" << json_result.str() << std::endl;

    std::cout << "[*] Transcoding From PSON..." << std::endl;
    std::cout << "[*] Transcodin Time: " << measure<>::execution([&]{json_transcoder.transcode_value();}) << " μs" << std::endl;
    std::cout << "\t" << json_transcoder.get_str() << std::endl;

    std::cout << "[*] Decoding From JSON..." << std::endl;
    protoson::json_decoder jsonDecoder(json_transcoder.get_str());
    protoson::pson value;
    jsonDecoder.parse(value);

    json_result.str(std::string());
    json_encoder.encode(value);
    parsed_json = json_result.str();
    std::cout << "\t" << parsed_json << std::endl;


    return 0;
}
