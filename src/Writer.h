#ifndef WRITER_H
#define WRITER_H

#include "vendor/ieee754_types.hpp"
#include <cstdint>
#include <fstream>
#include <vector>

typedef IEEE_754::_2008::Binary<32> float32_t;
typedef IEEE_754::_2008::Binary<64> float64_t;

class Writer
{
private:
    std::ofstream out;

public:
    Writer(const std::string &filename);

    void putBitVec(const std::vector<bool> &vec);
    void putBool(bool val);
    void putFloat32(float32_t val);
    void putFloat64(float64_t val);
    void putString(const std::string &val);
    void putUint8(uint8_t val);
    void putUint16(uint16_t val);
    void putUint32(uint32_t val);
    void putUint64(uint64_t val);
    void skipBytes(size_t len);
    void seekp(uint32_t pos);
    uint32_t tellp();
    void write(const char *val, size_t len);
};

#endif // WRITER_H
