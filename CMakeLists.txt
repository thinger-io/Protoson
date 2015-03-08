cmake_minimum_required(VERSION 2.8.4)
project(protoson)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES test/protoson.cpp src/pson.h src/util/json_encoder.hpp src/pson_encoder.hpp src/pson_decoder.hpp src/util/pson_json_transcoder.hpp src/util/json_decoder.hpp)
add_executable(protoson ${SOURCE_FILES})

set(JSON2PSON_FILES tools/json2pson.cpp)
add_executable(json2pson ${JSON2PSON_FILES})

set(PSON2JSON_FILES tools/pson2json.cpp)
add_executable(pson2json ${PSON2JSON_FILES})

set(TEST_FILES test/unit.cpp test/catch.hpp)
add_executable(unit ${TEST_FILES})

set(ENCODING_DECODING_EXAMPLE examples/encoding_decoding.cpp)
add_executable(encoding_decoding_example ${ENCODING_DECODING_EXAMPLE})

set(JSON_ENCODING examples/json_encoding.cpp)
add_executable(json_encoding ${JSON_ENCODING})

set(JSON_DECODING examples/json_decoding.cpp)
add_executable(json_decoding ${JSON_DECODING})