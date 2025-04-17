#include "Writer.h"

#include <bit>

namespace
{

template <typename NumberType, size_t byteCount>
void writeLittleEndian(std::ostream &out, NumberType val)
{
    for (size_t i = 0; i < byteCount; ++i) {
        out.put(0xff & val);
        val >>= 8;
    }
}

} // namespace

Writer::Writer(const std::string &filename) : out(filename, std::ios::binary) {}

void Writer::putBitVec(const std::vector<bool> &vec)
{
    uint8_t buf = 0;
    int offset = 0;
    for (bool val : vec) {
        if (val) {
            buf |= (1 << offset);
        }
        ++offset;
        if (offset >= 8) {
            out.put(buf);
            buf = 0;
            offset = 0;
        }
    }
    if (offset != 0) {
        out.put(buf);
    }
}

void Writer::putBool(bool val)
{
    out.put(val ? 1 : 0);
}

void Writer::putFloat32(float32_t val)
{
    if constexpr (std::endian::native == std::endian::big) {
        putUint32(std::bit_cast<std::uint32_t>(val));
    } else {
        out.write(reinterpret_cast<const char *>(&val), sizeof(float32_t));
    }
}

void Writer::putFloat64(float64_t val)
{
    if constexpr (std::endian::native == std::endian::big) {
        putUint64(std::bit_cast<std::uint64_t>(val));
    } else {
        out.write(reinterpret_cast<const char *>(&val), sizeof(float64_t));
    }
}

void Writer::putString(const std::string &val)
{
    // LEB128 encoded length prefix.
    size_t len = val.size();
    do {
        uint8_t b = len & 0x7f;
        len >>= 7;
        if (len != 0) {
            b |= 0x80;
        }
        out.put(b);
    } while (len != 0);
    out.write(val.c_str(), val.size());
}

void Writer::putUint8(uint8_t val)
{
    out.put(val);
}

void Writer::putUint16(uint16_t val)
{
    writeLittleEndian<uint16_t, 2>(out, val);
}

void Writer::putUint32(uint32_t val)
{
    writeLittleEndian<uint32_t, 4>(out, val);
}

void Writer::putUint64(uint64_t val)
{
    writeLittleEndian<uint64_t, 8>(out, val);
}

void Writer::skipBytes(size_t len)
{
    for (size_t i = 0; i < len; ++i) {
        out.put(0);
    }
}

void Writer::seekp(uint32_t pos)
{
    out.seekp(pos);
}

uint32_t Writer::tellp()
{
    return out.tellp();
}

void Writer::write(const char *val, size_t len)
{
    out.write(val, len);
}
