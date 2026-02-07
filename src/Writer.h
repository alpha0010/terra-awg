#ifndef WRITER_H
#define WRITER_H

#include "vendor/ieee754_types.hpp"
#include <bitset>
#include <cstdint>
#include <fstream>

typedef IEEE_754::_2008::Binary<32> float32_t;
typedef IEEE_754::_2008::Binary<64> float64_t;

class Writer
{
private:
    std::ofstream out;

public:
    Writer(const std::string &filename);

    template <size_t N> void putBitVec(std::bitset<N> vec)
    {
        std::bitset<N> mask(0xff);
        for (int i = (vec.size() + 7) / 8; i > 0; --i) {
            out.put((vec & mask).to_ulong());
            vec >>= 8;
        }
    }

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
