#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../src/pson.h"
#include "../src/util/json_encoder.hpp"

protoson::dynamic_memory_allocator alloc;
protoson::memory_allocator&protoson::pool = alloc;

using namespace protoson;
using namespace std;

TEST_CASE( "PSON-JSON Encoding", "[PSON-JSON]" ) {
    pson root;
    ostringstream out_stream;
    json_encoder encoder(out_stream);

    SECTION( "empty object" ) {
        pson_object & object = (pson_object &) root;
        encoder.encode(root);
        REQUIRE("{}"==out_stream.str());
    }

    SECTION( "empty array" ) {
        pson_array & object = (pson_array &) root;
        encoder.encode(root);
        REQUIRE("[]"==out_stream.str());
    }

    SECTION( "bool element" ) {
        root["true"] = true;
        root["false"] = false;
        encoder.encode(root);
        REQUIRE("{\"true\":true,\"false\":false}"==out_stream.str());
    }

    SECTION( "zero" ) {
        root["zero"] = 0;
        encoder.encode(root);
        REQUIRE("{\"zero\":0}"==out_stream.str());
    }

    SECTION( "one" ) {
        root["one"] = 1;
        encoder.encode(root);
        REQUIRE("{\"one\":1}"==out_stream.str());
    }

    SECTION( "one byte number" ) {
        root["number"] = 65;
        encoder.encode(root);
        REQUIRE("{\"number\":65}"==out_stream.str());
    }

    SECTION( "boundary number 127" ) {
        root["number"] = 127;
        encoder.encode(root);
        REQUIRE("{\"number\":127}"==out_stream.str());
    }

    SECTION( "boundary number 128" ) {
        root["number"] = 128;
        encoder.encode(root);
        REQUIRE("{\"number\":128}"==out_stream.str());
    }

    SECTION( "two byte number" ) {
        root["number"] = 329;
        encoder.encode(root);
        REQUIRE("{\"number\":329}"==out_stream.str());
    }

    SECTION( "three byte number" ) {
        root["number"] = 23900;
        encoder.encode(root);
        REQUIRE("{\"number\":23900}"==out_stream.str());
    }

    SECTION( "four byte number" ) {
        root["number"] = 9823909;
        encoder.encode(root);
        REQUIRE("{\"number\":9823909}"==out_stream.str());
    }

    SECTION( "uint64_t max" ) {
        root["number"] = std::numeric_limits<uint64_t>::max();
        encoder.encode(root);
        stringstream out;
        out << "{\"number\":" << std::numeric_limits<uint64_t>::max() << "}";
        REQUIRE(out.str()==out_stream.str());
    }

    SECTION( "int64_t min" ) {
        root["number"] = std::numeric_limits<int64_t>::min();
        encoder.encode(root);
        stringstream out;
        out << "{\"number\":" << std::numeric_limits<int64_t>::min() << "}";
        REQUIRE(out.str()==out_stream.str());
    }

    SECTION( "string" ) {
        root["string"] = "hello world!";
        encoder.encode(root);
        REQUIRE("{\"string\":\"hello world!\"}"==out_stream.str());
    }

    SECTION( "uft8" ) {
        root["utf8"] = "我能吞下玻璃而不伤身体。";
        encoder.encode(root);
        REQUIRE("{\"utf8\":\"我能吞下玻璃而不伤身体。\"}"==out_stream.str());
    }

    SECTION( "null" ) {
        root["null"];
        encoder.encode(root);
        REQUIRE("{\"null\":null}"==out_stream.str());
    }

    SECTION( "int-float" ) {
        root["float"] = 222.0;
        encoder.encode(root);
        REQUIRE("{\"float\":222}"==out_stream.str());
    }

    SECTION( "small-float" ) {
        root["float"] = 5.123f;
        encoder.encode(root);
        REQUIRE("{\"float\":5.123}"==out_stream.str());
    }

    SECTION( "small-double" ) {
        root["double"] = 5.1234;
        encoder.encode(root);
        REQUIRE("{\"double\":5.1234}"==out_stream.str());
    }

    SECTION( "small-neg-float" ) {
        root["float"] = -5.123f;
        encoder.encode(root);
        REQUIRE("{\"float\":-5.123}"==out_stream.str());
    }

    SECTION( "small-neg-double" ) {
        root["double"] = -5.1234;
        encoder.encode(root);
        REQUIRE("{\"double\":-5.1234}"==out_stream.str());
    }
}

