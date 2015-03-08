[![Build Status](https://travis-ci.org/thinger-io/Protoson.svg)](https://travis-ci.org/thinger-io/Protoson)
[![Coverage Status](https://coveralls.io/repos/thinger-io/Protoson/badge.svg?branch=master)](https://coveralls.io/r/thinger-io/Protoson?branch=master)

Protoson is a library for encoding and decoding unstructured data in a binary format. Similar to JSON but fast and small. Protoson is based on some Protocol Buffers encoding techniques, but it is not compatible over the wire. However you can embed the protoson output y any Protocol Buffers bytes field. Actually you can transmit the raw bytes as you like, as it does not require Protocol Buffers itself.

## Design goals

- **Small compiled code size**. This library has been mainly designed for microcontrollers or devices with very limited resources. The whole library takes less than 3.5KB on an Arduino. It does not require any STL structure.

- **Custom memory allocators**. Protoson C++ can use different memory allocations approaches. Currently there are implemented a circular, and a dynamic memory allocator. Choose your allocation scheme according to your needs.

- **Small output**. The output size is comparable to the well-known MessagePack. Depending on the encoded data it can be even smaller.

- **More than JSON**. Protoson can encode binary data and any other type in the root, without explicitly requiring a parent object or array.