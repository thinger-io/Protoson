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
    std::ostringstream stream_;
public:
    json_transcoder(char *buffer) : protoson::pson_json_transcoder(stream_), buffer_(buffer)
    {

    }

    std::string get_str(){
        return stream_.str();
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

    // adding some basic types
    object["hello"] = "world!";
    object["time"] = 1234567890;
    object["float"] = 0.01234;
    object["boolean"] = true;
    object["otherbool"] = false;
    object["null"].set_null();
    // adding object in array
    protoson::pson_object & array_object = object["obj"];
    array_object["what"] = "that";

    // adding array
    protoson::pson_array & array = object["arr"];

    // adding values in array
    array.add(1);
    array.add(2);
    array.add(3);

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
