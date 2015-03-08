cmake_minimum_required(VERSION 2.8.4)
project(protoson)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(protoson test/protoson.cpp src/pson.h src/pson_encoder.hpp src/pson_decoder.hpp src/util/json_encoder.hpp src/util/json_decoder.hpp src/util/pson_json_transcoder.hpp)
#add_executable(json2pson tools/json2pson.cpp src/pson.h src/pson_encoder.hpp src/util/json_decoder.hpp)
#add_executable(pson2json tools/pson2json.cpp src/pson.h src/pson_decoder.hpp src/util/json_encoder.hpp)
#add_executable(unit test/unit.cpp src/util/json_encoder.hpp test/catch.hpp)
#add_executable(encoding_decoding_example examples/encoding_decoding.cpp src/pson.h src/pson_encoder.hpp src/pson_decoder.hpp)
#add_executable(json_encoding examples/json_encoding.cpp src/pson.h src/util/json_encoder.hpp)
#add_executable(json_decoding examples/json_decoding.cpp src/pson.h src/util/json_decoder.hpp)