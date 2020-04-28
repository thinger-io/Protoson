#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file

#include "catch.hpp"
#include "../src/pson.h"
#include "../src/util/json_encoder.hpp"

protoson::dynamic_memory_allocator alloc;
protoson::memory_allocator&protoson::pool = alloc;

using namespace protoson;
using namespace std;

TEST_CASE( "PSON Reading", "[PSON-JSON]" ) {
    pson object;

    SECTION("true value") {
        object = true;
        REQUIRE((bool)object==true);
    }

    SECTION("false value") {
        object = false;
        REQUIRE((bool)object==false);
    }

    SECTION("zero value") {
        object = 0;
        REQUIRE((int)object==0);
    }

    SECTION("one value") {
        object = 1;
        REQUIRE((int)object==1);
    }

    SECTION("number value") {
        object = 55;
        REQUIRE((int)object==55);
    }

    SECTION("negative number value") {
        object = -55;
        REQUIRE((int)object==-55);
    }

    SECTION("float value") {
        object = 555.66f;
        REQUIRE((float)object==555.66f);
    }

    SECTION("double value") {
        object = 555.66;
        REQUIRE((double)object==555.66);
    }

    SECTION("string value") {
        object = "hello";
        REQUIRE(strcmp((const char*)object,"hello")==0);
    }

    SECTION("bytes value") {
        uint8_t bytes[5] = {55, 56, 57, 58, 59};
        object.set_bytes(bytes, 5);
        uint8_t * data;
        size_t size;
        object.get_bytes(data, size);
        REQUIRE(size==5);
        for(size_t i=0; i<size; i++){
            REQUIRE(((uint8_t*)data)[i]==55+i);
        }
    }

    SECTION("array values"){
        pson_array& array = object;
        array.add(true);
        array.add(5);
        array.add(555.66f);
        array.add("hello");
        pson_array::iterator it = array.begin();
        REQUIRE(true == (bool)it.item());
        it.next();
        REQUIRE(5 == (int)it.item());
        it.next();
        REQUIRE(555.66f == (float)it.item());
        it.next();
        REQUIRE(strcmp("hello", (const char *)it.item())==0);
    }
}

TEST_CASE( "PSON Introspection", "[PSON-JSON]" ) {
    pson object;

    SECTION("null value") {
        object.set_null();
        REQUIRE(object.is_null());
        REQUIRE(!object.is_number());
        REQUIRE(!object.is_boolean());
        REQUIRE(!object.is_object());
        REQUIRE(!object.is_array());
        REQUIRE(!object.is_bytes());
        REQUIRE(!object.is_string());
    }

    SECTION("bool value") {
        object = true;
        REQUIRE(!object.is_null());
        REQUIRE(!object.is_number());
        REQUIRE(object.is_boolean());
        REQUIRE(!object.is_object());
        REQUIRE(!object.is_array());
        REQUIRE(!object.is_bytes());
        REQUIRE(!object.is_string());
    }

    SECTION("number value") {
        object = 55;
        REQUIRE(!object.is_null());
        REQUIRE(object.is_number());
        REQUIRE(!object.is_boolean());
        REQUIRE(!object.is_object());
        REQUIRE(!object.is_array());
        REQUIRE(!object.is_bytes());
        REQUIRE(!object.is_string());
    }

    SECTION("float value") {
        object = 555.66;
        REQUIRE(!object.is_null());
        REQUIRE(object.is_number());
        REQUIRE(!object.is_boolean());
        REQUIRE(!object.is_object());
        REQUIRE(!object.is_array());
        REQUIRE(!object.is_bytes());
        REQUIRE(!object.is_string());
    }

    SECTION("string value") {
        object = "hello";
        REQUIRE(!object.is_null());
        REQUIRE(!object.is_number());
        REQUIRE(!object.is_boolean());
        REQUIRE(!object.is_object());
        REQUIRE(!object.is_array());
        REQUIRE(!object.is_bytes());
        REQUIRE(object.is_string());
    }

    SECTION("object value") {
        pson_object& obj = object;
        REQUIRE(!object.is_null());
        REQUIRE(!object.is_number());
        REQUIRE(!object.is_boolean());
        REQUIRE(object.is_object());
        REQUIRE(!object.is_array());
        REQUIRE(!object.is_bytes());
        REQUIRE(!object.is_string());
    }

    SECTION("array value") {
        pson_array& array = object;
        REQUIRE(!object.is_null());
        REQUIRE(!object.is_number());
        REQUIRE(!object.is_boolean());
        REQUIRE(!object.is_object());
        REQUIRE(object.is_array());
        REQUIRE(!object.is_bytes());
        REQUIRE(!object.is_string());
    }

    SECTION("bytes value") {
        uint8_t bytes[4] = {55, 55, 55, 55};
        object.set_bytes(bytes, 4);
        REQUIRE(!object.is_null());
        REQUIRE(!object.is_number());
        REQUIRE(!object.is_boolean());
        REQUIRE(!object.is_object());
        REQUIRE(!object.is_array());
        REQUIRE(object.is_bytes());
        REQUIRE(!object.is_string());
    }
}

TEST_CASE( "PSON-JSON Encoding", "[PSON-JSON]" ) {
    pson root;
    ostringstream out_stream;
    json_encoder encoder(out_stream);

    SECTION("null value") {
        root.set_null();
        encoder.encode(root);
        REQUIRE("null" == out_stream.str());
    }

    SECTION("true value") {
        root = true;
        encoder.encode(root);
        REQUIRE("true" == out_stream.str());
    }

    SECTION("false value") {
        root = false;
        encoder.encode(root);
        REQUIRE("false" == out_stream.str());
    }

    SECTION("zero value") {
        root = 0;
        encoder.encode(root);
        REQUIRE("0" == out_stream.str());
    }

    SECTION("one value") {
        root = 1;
        encoder.encode(root);
        REQUIRE("1" == out_stream.str());
    }

    SECTION("int value") {
        root = 225;
        encoder.encode(root);
        REQUIRE("225" == out_stream.str());
    }

    SECTION("signed int value") {
        root = -225;
        encoder.encode(root);
        REQUIRE("-225" == out_stream.str());
    }

    SECTION("float value") {
        root = 225.33f;
        encoder.encode(root);
        REQUIRE("225.33" == out_stream.str());
    }

    SECTION("signed float value") {
        root = -225.33f;
        encoder.encode(root);
        REQUIRE("-225.33" == out_stream.str());
    }

    SECTION("signed double value") {
        root = 225.33;
        encoder.encode(root);
        REQUIRE("225.33" == out_stream.str());
    }

    SECTION("signed double value") {
        root = -225.33;
        encoder.encode(root);
        REQUIRE("-225.33" == out_stream.str());
    }

    SECTION("uint64_t max value") {
        root = std::numeric_limits<uint64_t>::max();
        encoder.encode(root);
        stringstream out;
        out << std::numeric_limits<uint64_t>::max();
        REQUIRE(out.str() == out_stream.str());
    }

    SECTION("int64_t min value") {
        root = std::numeric_limits<int64_t>::min();
        encoder.encode(root);
        stringstream out;
        out << std::numeric_limits<int64_t>::min();
        REQUIRE(out.str() == out_stream.str());
    }

    SECTION("int-float value") {
        root["float"] = 222.0;
        encoder.encode(root);
        REQUIRE("{\"float\":222}" == out_stream.str());
    }

    SECTION("string value") {
        root = "test";
        encoder.encode(root);
        REQUIRE("\"test\"" == out_stream.str());
    }

    SECTION("uft8 string value") {
        root = "我能吞下玻璃而不伤身体。";
        encoder.encode(root);
        REQUIRE("\"我能吞下玻璃而不伤身体。\"" == out_stream.str());
    }

    SECTION("empty object") {
        pson_object &object = (pson_object &) root;
        encoder.encode(root);
        REQUIRE("{}" == out_stream.str());
    }

    SECTION("empty array") {
        pson_array &array = (pson_array &) root;
        encoder.encode(root);
        REQUIRE("[]" == out_stream.str());
    }

    SECTION("object with array") {
        pson_array& array = root["array"];
        encoder.encode(root);
        REQUIRE("{\"array\":[]}" == out_stream.str());
    }

    SECTION("object with object") {
        pson_object& object = root["object"];
        encoder.encode(root);
        REQUIRE("{\"object\":{}}" == out_stream.str());
    }

    SECTION("object with multiple elements") {
        root["one"] = 1;
        root["true"] = true;
        root["str"] = "str";
        root["float"] = 33.44f;
        encoder.encode(root);
        REQUIRE("{\"one\":1,\"true\":true,\"str\":\"str\",\"float\":33.44}" == out_stream.str());
    }

    SECTION("array with multiple elements") {
        pson_array &array = (pson_array &) root;
        array.add(1);
        array.add(true);
        array.add("str");
        array.add(33.44f);
        encoder.encode(root);
        REQUIRE("[1,true,\"str\",33.44]" == out_stream.str());
    }

    SECTION("array with object") {
        pson_array& array = (pson_array &) root;
        pson_object& object = array.add_object();
        encoder.encode(root);
        REQUIRE("[{}]" == out_stream.str());
    }

    SECTION("array with object") {
        pson_array& array = (pson_array &) root;
        pson_object& object = array.add_object();
        object["key"] = 5;
        encoder.encode(root);
        REQUIRE("[{\"key\":5}]" == out_stream.str());
    }

    SECTION("array with array") {
        pson_array& array = (pson_array &) root;
        pson_array& sub_array = array.add_array();
        encoder.encode(root);
        REQUIRE("[[]]" == out_stream.str());
    }

    SECTION("array with array") {
        pson_array& array = (pson_array &) root;
        pson_array& sub_array = array.add_array();
        sub_array.add(5);
        encoder.encode(root);
        REQUIRE("[[5]]" == out_stream.str());
    }
}