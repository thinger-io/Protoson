// The MIT License (MIT)
//
// Copyright (c) 2017 THINK BIG LABS S.L.
// Author: alvarolb@gmail.com (Alvaro Luis Bustamante)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef JSON_DECODER_HPP
#define JSON_DECODER_HPP

#include <stdexcept>
#include <string>
#include <cmath>
#include "json.hpp"
#include "../pson.h"

namespace nlohmann
{

    static void to_pson_internal(const json& j, protoson::pson& p)
    {
        switch (j.type())
        {
            case detail::value_t::null:
            {
                // nil
                p.set_null();
                break;
            }

            case detail::value_t::boolean:
            {
                // true and false
                p = j.get<bool>();
                break;
            }

            case detail::value_t::number_integer:
            {
                p = j.get<std::int64_t>();
                break;
            }

            case detail::value_t::number_unsigned:
            {
                p = j.get<std::uint64_t>();
                break;
            }

            case detail::value_t::number_float:
            {
                p = j.get<double>();
                break;
            }

            case detail::value_t::string:
            {
                p = j.get<std::string>();
                break;
            }

            case detail::value_t::array:
            {
                protoson::pson_array& array = (protoson::pson_array&) p;
                // append each element
                for (const auto& el : j)
                {
                    to_pson_internal(el, *array.create_item());
                }
                break;
            }

            case detail::value_t::object:
            {
                protoson::pson_object& object = (protoson::pson_object&) p;
                // append each element
                for (json::const_iterator it = j.begin(); it != j.end(); ++it) {
                    to_pson_internal(it.value(), object[it.key().c_str()]);
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }

    static void to_json_internal(protoson::pson& p, json& j)
    {
        switch(p.get_type())
        {
            case protoson::pson::zero_field:
                j = 0;
                break;
            case protoson::pson::one_field:
                j = 1;
                break;
            case protoson::pson::true_field:
                j = true;
                break;
            case protoson::pson::false_field:
                j = false;
                break;
            case protoson::pson::svarint_field:
                j = (int64_t) p;
                break;
            case protoson::pson::varint_field:
                j = (uint64_t) p;
                break;
            case protoson::pson::float_field:
            {
                float val = (float) p;
                if(std::isnan(val)){
                    j = nullptr;
                }else{
                    j = val;
                }
            }
                break;
            case protoson::pson::double_field:
            {
                float val = (double) p;
                if(std::isnan(val)){
                    j = nullptr;
                }else{
                    j = val;
                }
            }
                break;
            case protoson::pson::string_field:
                j = (const char*) p;
                break;
            case protoson::pson::empty_string:
            case protoson::pson::empty_bytes:
                j = "";
                break;
            case protoson::pson::object_field:
            {
                j = json::object();
                protoson::pson_container<protoson::pson_pair>::iterator it = ((protoson::pson_object&)p).begin();
                while(it.valid()){
                    to_json_internal(it.item().value(), j[it.item().name()]);
                    it.next();
                }
            }
                break;
            case protoson::pson::array_field:
            {
                j = json::array();
                protoson::pson_container<protoson::pson>::iterator it = ((protoson::pson_array&)p).begin();
                while(it.valid()){
                    json array_value;
                    to_json_internal(it.item(), array_value);
                    j.push_back(array_value);
                    it.next();
                }
            }
                break;

            case protoson::pson::bytes_field:
            {
                uint8_t * data = NULL;
                size_t size = 0;
                p.get_bytes(data, size);
                // TODO JSON library will add support for binary types in next release
                // https://github.com/nlohmann/json/issues/483
                j = "";
            }
                break;
            case protoson::pson::null_field:
                j = nullptr;
                break;
            case protoson::pson::empty:
                j = json();
                break;
        }
    }

    static void to_pson(const json& j, protoson::pson& p)
    {
        to_pson_internal(j, p);
    }

    static void to_json(protoson::pson& p, json& j)
    {
        to_json_internal(p, j);
    }
}

namespace protoson{
    class json_decoder{
    public:
        static bool parse(const std::string& json, pson& pson){
            try{
                nlohmann::json json_parsed = nlohmann::json::parse(json);
                nlohmann::to_pson(json_parsed, pson);
            }catch(...){
                return false;
            }
            return true;
        }

        static bool parse(const nlohmann::json& json, pson& pson){
            try{
                nlohmann::to_pson(json, pson);
            }catch(...){
                return false;
            }
            return true;
        }

        static bool to_json(pson& pson, nlohmann::json& json){
            try{
                nlohmann::to_json(pson, json);
            }catch(...){
                return false;
            }
            return true;
        }
    };
}

#endif