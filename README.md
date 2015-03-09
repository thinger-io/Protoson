[![Build Status](https://travis-ci.org/thinger-io/Protoson.svg)](https://travis-ci.org/thinger-io/Protoson)

Protoson is a library for encoding and decoding unstructured data in a binary format. Similar to JSON but fast and small. Protoson is based on some Protocol Buffers encoding techniques, but it is not compatible over the wire. However you can embed the protoson output in any Protocol Buffers bytes field. Actually you can transmit the raw bytes as you like, as it does not require Protocol Buffers itself.

## Design goals

- **Small compiled code size**. This library has been mainly designed for microcontrollers or devices with very limited resources. The whole library takes less than 3.5KB on an Arduino. It does not require any STL structure.

- **Custom memory allocators**. Protoson C++ can use different memory allocations approaches. Currently there are implemented a circular, and a dynamic memory allocator. Choose your allocation scheme according to your needs.

- **Small output**. The output size is comparable to the well-known [MessagePack](http://msgpack.org/). Depending on the encoded data it can be even smaller.

- **More than JSON**. Protoson can encode binary data and other data types in the root, without explicitly requiring a parent object or array.

## Integration

The single required source, `pson.h` file is in the `src` directory. All you need to do is add this file and define the memory allocator. The memory allocator must be defined only once in a single `.cpp` file.

```cpp
#include "pson.h"
protoson::dynamic_memory_allocator alloc;
protoson::memory_allocator& protoson::pool = alloc;
```

## Quick Example

```cpp
#include "pson.h"
protoson::dynamic_memory_allocator alloc;
protoson::memory_allocator& protoson::pool = alloc;
using namespace protoson;

int main(){
    pson object;
    object["key1"] = 55;
    object["key2"] = true;
    object["key3"] = "hello";
    object["key4"] = 3.14;
}
```

## Serialization/Deserialization

Serialization and deserialization is done over some virtual classes that provide raw methods for writing and reading bytes. In this way you can create your own wrappers to serialize/deserialize to/from multiple data sources, like memory, socket, file, etc. In the following there are two  classes that allows serialization and deserialization to a memory buffer.

```cpp
// helper class for encoding to memory
// you can create your own encoders writing to file, socket, etc)
class memory_writer : public pson_encoder {
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

// helper class for decoding form memory
class memory_reader : public pson_decoder {
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

```

Now you can use these customs classes in the following way:

```cpp
// reserve memory buffer
char memory_buffer[512];

// create a sample object
pson object;
object["hello"] = "world";
object["value"] = 336;

// encode
memory_writer writer(memory_buffer);
writer.encode(object);

// decode
memory_reader reader(memory_buffer);
pson decoded_object;
reader.decode(decoded_object);

// use the values
const char* str = decoded_object["hello"];
int value = decoded_object["value"];
```

## Memory Allocators

In some environments with limited memory or without dynamic memory allocation can be useful to define custom memory allocators. Protoson requires memory for storing the data structure in memory, i.e., when your are building a object, or decoding it from some source. Encoding and Decoding part does not require memory itself.

Currently you can switch between two different memory  allocators: `circular_memory_allocator` and `dynamic_memory_allocator`:

Use a `dynamic_memory_allocator` if you want to/can use dynamic memory. Internally, the dynamic memory allocator uses `malloc` and `free`.

```cpp
#include "pson.h"
protoson::dynamic_memory_allocator alloc;
protoson::memory_allocator& protoson::pool = alloc;
```

Use a `circular_memory_allocator` if you want to define a static memory buffer. In this case you need to specify a buffer size that will depend on your maximum message length. Notice that the required buffer size can vary between platforms for storing the same message, depending on the pointer size (16, 32, or 64 bits), memory alignment, padding, etc.

```cpp
#include "pson.h"
protoson::circular_memory_allocator<512> alloc;
protoson::memory_allocator& protoson::pool = alloc;
```

## TODO

Protoson is currently being developed and should not be integrated in production code until it is more tested. Some things that requires attention now are:

 - Handle errors in some way while encoding and decoding (without using exceptions)
 - Complete unitary tests
 - Avoid memory leaks when reassigning values to a `pson` object.
 - Complete examples 
 - Document code

## License

<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

The class is licensed under the [MIT License](http://opensource.org/licenses/MIT):

Copyright &copy; 2015 [THINGER LTD](http://thinger.io)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.