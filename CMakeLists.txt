cmake_minimum_required(VERSION 2.8.4)
project(protoson)

# add c++11 support for json library
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")

## build unit test
add_executable(pson_unit test/unit.cpp src/util/json_encoder.hpp test/catch.hpp)
add_executable(pson_binary test/binary.cpp src/util/json_decoder.hpp)

# build command line tools
add_executable(json2pson tools/json2pson.cpp src/pson.h src/util/json_decoder.hpp)
add_executable(pson2json tools/pson2json.cpp src/pson.h src/util/json_encoder.hpp)
add_executable(pson_test_file tools/pson_test_file.cpp src/pson.h src/util/json_encoder.hpp)

# build examples
add_executable(complete examples/complete.cpp src/pson.h src/util/json_encoder.hpp src/util/json_decoder.hpp)
add_executable(pson_enc_dec examples/pson_enc_dec.cpp src/pson.h)
add_executable(json_encoding examples/json_encoding.cpp src/pson.h src/util/json_encoder.hpp)
add_executable(json_decoding examples/json_decoding.cpp src/pson.h src/util/json_decoder.hpp)