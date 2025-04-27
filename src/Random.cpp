#include "Random.h"

#include "Util.h"
#include "vendor/OpenSimplexNoise.hpp"
#include <immintrin.h>
#include <iostream>
#include <smmintrin.h>

inline __m128i SIMD_ps_to_i32(__m128 a)
{
    return _mm_cvtps_epi32(
        _mm_round_ps(a, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC));
}

__m128 OpenSimplexNoise::Evaluate_SIMD(__m128 x, __m128 y, __m128 z, __m128 w)
{
    // double stretchOffset = (x + y + z + w) * STRETCH_4D;
    __m128 stretchOffset = _mm_mul_ps(
        _mm_add_ps(_mm_add_ps(x, y), _mm_add_ps(z, w)),
        _mm_set_ps1(STRETCH_4D));
    // double xs = x + stretchOffset;
    __m128 xs = _mm_add_ps(x, stretchOffset);
    // double ys = y + stretchOffset;
    __m128 ys = _mm_add_ps(y, stretchOffset);
    // double zs = z + stretchOffset;
    __m128 zs = _mm_add_ps(z, stretchOffset);
    // double ws = w + stretchOffset;
    __m128 ws = _mm_add_ps(w, stretchOffset);

    // int xsb = FastFloor(xs);
    __m128 xsb = _mm_round_ps(xs, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
    // int ysb = FastFloor(ys);
    __m128 ysb = _mm_round_ps(ys, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
    // int zsb = FastFloor(zs);
    __m128 zsb = _mm_round_ps(zs, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);
    // int wsb = FastFloor(ws);
    __m128 wsb = _mm_round_ps(ws, _MM_FROUND_TO_NEG_INF | _MM_FROUND_NO_EXC);

    // double squishOffset = (xsb + ysb + zsb + wsb) * SQUISH_4D;
    __m128 squishOffset = _mm_mul_ps(
        _mm_add_ps(_mm_add_ps(xsb, ysb), _mm_add_ps(zsb, wsb)),
        _mm_set_ps1(SQUISH_4D));
    // double dx0 = x - (xsb + squishOffset);
    __m128 dx0 = _mm_sub_ps(x, _mm_add_ps(xsb, squishOffset));
    // double dy0 = y - (ysb + squishOffset);
    __m128 dy0 = _mm_sub_ps(y, _mm_add_ps(ysb, squishOffset));
    // double dz0 = z - (zsb + squishOffset);
    __m128 dz0 = _mm_sub_ps(z, _mm_add_ps(zsb, squishOffset));
    // double dw0 = w - (wsb + squishOffset);
    __m128 dw0 = _mm_sub_ps(w, _mm_add_ps(wsb, squishOffset));

    // double xins = xs - xsb;
    __m128 xins = _mm_sub_ps(xs, xsb);
    // double yins = ys - ysb;
    __m128 yins = _mm_sub_ps(ys, ysb);
    // double zins = zs - zsb;
    __m128 zins = _mm_sub_ps(zs, zsb);
    // double wins = ws - wsb;
    __m128 wins = _mm_sub_ps(ws, wsb);

    // double inSum = xins + yins + zins + wins;
    __m128 inSum = _mm_add_ps(_mm_add_ps(xins, yins), _mm_add_ps(zins, wins));

    /*
    int hash = static_cast<int>(zins - wins + 1) |
               static_cast<int>(yins - zins + 1) << 1 |
               static_cast<int>(yins - wins + 1) << 2 |
               static_cast<int>(xins - yins + 1) << 3 |
               static_cast<int>(xins - zins + 1) << 4 |
               static_cast<int>(xins - wins + 1) << 5 |
               static_cast<int>(inSum) << 6 |
               static_cast<int>(inSum + wins) << 8 |
               static_cast<int>(inSum + zins) << 11 |
               static_cast<int>(inSum + yins) << 14 |
               static_cast<int>(inSum + xins) << 17;
    */
    __m128 val1 = _mm_set_ps1(1);
    __m128i hash = _mm_or_si128(
        _mm_or_si128(
            _mm_or_si128(
                _mm_or_si128(
                    SIMD_ps_to_i32(_mm_add_ps(_mm_sub_ps(zins, wins), val1)),
                    _mm_slli_epi32(
                        SIMD_ps_to_i32(
                            _mm_add_ps(_mm_sub_ps(yins, zins), val1)),
                        1)),
                _mm_or_si128(
                    _mm_slli_epi32(
                        SIMD_ps_to_i32(
                            _mm_add_ps(_mm_sub_ps(yins, wins), val1)),
                        2),
                    _mm_slli_epi32(
                        SIMD_ps_to_i32(
                            _mm_add_ps(_mm_sub_ps(xins, yins), val1)),
                        3))),
            _mm_or_si128(
                _mm_or_si128(
                    _mm_slli_epi32(
                        SIMD_ps_to_i32(
                            _mm_add_ps(_mm_sub_ps(xins, zins), val1)),
                        4),
                    _mm_slli_epi32(
                        SIMD_ps_to_i32(
                            _mm_add_ps(_mm_sub_ps(xins, wins), val1)),
                        5)),
                _mm_or_si128(
                    _mm_slli_epi32(SIMD_ps_to_i32(inSum), 6),
                    _mm_slli_epi32(
                        SIMD_ps_to_i32(_mm_add_ps(inSum, wins)),
                        8)))),
        _mm_or_si128(
            _mm_or_si128(
                _mm_slli_epi32(SIMD_ps_to_i32(_mm_add_ps(inSum, zins)), 11),
                _mm_slli_epi32(SIMD_ps_to_i32(_mm_add_ps(inSum, yins)), 14)),
            _mm_slli_epi32(SIMD_ps_to_i32(_mm_add_ps(inSum, xins)), 17)));

    // Contribution4 *c = lookup4D[hash];
    std::array<Contribution4 *, 4> c{
        lookup4D[_mm_extract_epi32(hash, 0)],
        lookup4D[_mm_extract_epi32(hash, 1)],
        lookup4D[_mm_extract_epi32(hash, 2)],
        lookup4D[_mm_extract_epi32(hash, 3)],
    };

    // double value = 0.0;
    __m128 value = _mm_setzero_ps();

    float xsbBuf[4];
    _mm_storeu_ps(xsbBuf, xsb);
    float ysbBuf[4];
    _mm_storeu_ps(ysbBuf, ysb);
    float zsbBuf[4];
    _mm_storeu_ps(zsbBuf, zsb);
    float wsbBuf[4];
    _mm_storeu_ps(wsbBuf, wsb);
    while (c[0] != nullptr && c[1] != nullptr && c[2] != nullptr &&
           c[3] != nullptr) {
        // double dx = dx0 + c->dx;
        __m128 dx =
            _mm_add_ps(dx0, _mm_set_ps(c[3]->dx, c[2]->dx, c[1]->dx, c[0]->dx));
        // double dy = dy0 + c->dy;
        __m128 dy =
            _mm_add_ps(dy0, _mm_set_ps(c[3]->dy, c[2]->dy, c[1]->dy, c[0]->dy));
        // double dz = dz0 + c->dz;
        __m128 dz =
            _mm_add_ps(dz0, _mm_set_ps(c[3]->dz, c[2]->dz, c[1]->dz, c[0]->dz));
        // double dw = dw0 + c->dw;
        __m128 dw =
            _mm_add_ps(dw0, _mm_set_ps(c[3]->dw, c[2]->dw, c[1]->dw, c[0]->dw));

        // double attn = 2 - dx * dx - dy * dy - dz * dz - dw * dw;
        __m128 attn = _mm_sub_ps(
            _mm_set_ps1(2),
            _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(dx, dx), _mm_mul_ps(dy, dy)),
                _mm_add_ps(_mm_mul_ps(dz, dz), _mm_mul_ps(dw, dw))));
        /*
        if (attn > 0) {
            int px = xsb + c->xsb;
            int py = ysb + c->ysb;
            int pz = zsb + c->zsb;
            int pw = wsb + c->wsb;

            int i = perm4D
                [(perm[(perm[(perm[px & 0xFF] + py) & 0xFF] + pz) & 0xFF] +
                  pw) &
                 0xFF];
            double valuePart = gradients4D[i] * dx + gradients4D[i + 1] * dy +
                               gradients4D[i + 2] * dz +
                               gradients4D[i + 3] * dw;

            attn *= attn;
            value += attn * attn * valuePart;
        }
        c = c->Next;
        */
        float attnBuf[4];
        _mm_storeu_ps(attnBuf, attn);
        float dxBuf[4];
        _mm_storeu_ps(dxBuf, dx);
        float dyBuf[4];
        _mm_storeu_ps(dyBuf, dy);
        float dzBuf[4];
        _mm_storeu_ps(dzBuf, dz);
        float dwBuf[4];
        _mm_storeu_ps(dwBuf, dw);
        float valuePartBuf[] = {0, 0, 0, 0};
        for (int idx = 0; idx < 4; ++idx) {
            if (attnBuf[idx] > 0) {
                int px = xsbBuf[idx] + c[idx]->xsb;
                int py = ysbBuf[idx] + c[idx]->ysb;
                int pz = zsbBuf[idx] + c[idx]->zsb;
                int pw = wsbBuf[idx] + c[idx]->wsb;

                int i = perm4D
                    [(perm[(perm[(perm[px & 0xFF] + py) & 0xFF] + pz) & 0xFF] +
                      pw) &
                     0xFF];
                valuePartBuf[idx] = gradients4D[i] * dxBuf[idx] +
                                    gradients4D[i + 1] * dyBuf[idx] +
                                    gradients4D[i + 2] * dzBuf[idx] +
                                    gradients4D[i + 3] * dwBuf[idx];
            }
            c[idx] = c[idx]->Next;
        }
        __m128 attnSq = _mm_mul_ps(attn, attn);
        value = _mm_add_ps(
            value,
            _mm_mul_ps(_mm_mul_ps(attnSq, attnSq), _mm_loadu_ps(valuePartBuf)));
    }

    float dx0Buf[4];
    _mm_storeu_ps(dx0Buf, dx0);
    float dy0Buf[4];
    _mm_storeu_ps(dy0Buf, dy0);
    float dz0Buf[4];
    _mm_storeu_ps(dz0Buf, dz0);
    float dw0Buf[4];
    _mm_storeu_ps(dw0Buf, dw0);
    float valueDeltaBuf[] = {0, 0, 0, 0};
    for (int idx = 0; idx < 4; ++idx) {
        while (c[idx] != nullptr) {
            float dx = dx0Buf[idx] + c[idx]->dx;
            float dy = dy0Buf[idx] + c[idx]->dy;
            float dz = dz0Buf[idx] + c[idx]->dz;
            float dw = dw0Buf[idx] + c[idx]->dw;

            float attn = 2 - dx * dx - dy * dy - dz * dz - dw * dw;
            if (attn > 0) {
                int px = xsbBuf[idx] + c[idx]->xsb;
                int py = ysbBuf[idx] + c[idx]->ysb;
                int pz = zsbBuf[idx] + c[idx]->zsb;
                int pw = wsbBuf[idx] + c[idx]->wsb;

                int i = perm4D
                    [(perm[(perm[(perm[px & 0xFF] + py) & 0xFF] + pz) & 0xFF] +
                      pw) &
                     0xFF];
                float valuePart =
                    gradients4D[i] * dx + gradients4D[i + 1] * dy +
                    gradients4D[i + 2] * dz + gradients4D[i + 3] * dw;

                attn *= attn;
                valueDeltaBuf[idx] += attn * attn * valuePart;
            }
            c[idx] = c[idx]->Next;
        }
    }
    value = _mm_add_ps(value, _mm_loadu_ps(valueDeltaBuf));

    // return value * NORM_4D;
    return _mm_mul_ps(value, _mm_set_ps1(NORM_4D));
}

Random::Random()
    : noiseWidth(0), noiseHeight(0), noiseDeltaX(0), noiseDeltaY(0),
      rnd(std::random_device{}())
{
}

void Random::initNoise(int width, int height, double scale)
{
    std::cout << "Sampling noise\n";
    fineNoise.resize(width * height);
    coarseNoise.resize(width * height);
    std::uniform_int_distribution<int64_t> dist(
        0,
        std::numeric_limits<int64_t>::max());
    OpenSimplexNoise noise{dist(rnd)};
    double radiusX = scale * width * 0.5 * std::numbers::inv_pi;
    double radiusY = scale * height * 0.5 * std::numbers::inv_pi;
    parallelFor(
        std::views::iota(0, width),
        [width, height, radiusX, radiusY, &noise, this](int x) {
            float tX = 2 * std::numbers::pi_v<float> * x / width;
            __m128 x1 = _mm_set_ps1(radiusX * std::cos(tX));
            __m128 x2 = _mm_set_ps1(radiusX * std::sin(tX));
            __m128 val0125 = _mm_set_ps1(0.125);
            __m128 val025 = _mm_set_ps1(0.25);
            __m128 val05 = _mm_set_ps1(0.5);
            __m128 val2 = _mm_set_ps1(2);
            __m128 val4 = _mm_set_ps1(4);
            for (int y = 0; y < height; y += 4) {
                float tY[] = {
                    2 * std::numbers::pi_v<float> * (y + 3) / height,
                    2 * std::numbers::pi_v<float> * (y + 2) / height,
                    2 * std::numbers::pi_v<float> * (y + 1) / height,
                    2 * std::numbers::pi_v<float> * y / height};
                __m128 y1 = _mm_set_ps(
                    radiusY * std::cos(tY[0]),
                    radiusY * std::cos(tY[1]),
                    radiusY * std::cos(tY[2]),
                    radiusY * std::cos(tY[3]));
                __m128 y2 = _mm_set_ps(
                    radiusY * std::sin(tY[0]),
                    radiusY * std::sin(tY[1]),
                    radiusY * std::sin(tY[2]),
                    radiusY * std::sin(tY[3]));
                __m128 fineNoiseSampls = _mm_add_ps(
                    _mm_add_ps(
                        noise.Evaluate_SIMD(x1, x2, y1, y2),
                        _mm_mul_ps(
                            val05,
                            noise.Evaluate_SIMD(
                                _mm_mul_ps(val2, x1),
                                _mm_mul_ps(val2, x2),
                                _mm_mul_ps(val2, y1),
                                _mm_mul_ps(val2, y2)))),
                    _mm_mul_ps(
                        val025,
                        noise.Evaluate_SIMD(
                            _mm_mul_ps(val4, x1),
                            _mm_mul_ps(val4, x2),
                            _mm_mul_ps(val4, y1),
                            _mm_mul_ps(val4, y2))));
                _mm_storeu_ps(
                    fineNoise.data() + x * height + y,
                    fineNoiseSampls);
                _mm_storeu_ps(
                    coarseNoise.data() + x * height + y,
                    _mm_add_ps(
                        _mm_add_ps(
                            noise.Evaluate_SIMD(
                                _mm_mul_ps(val0125, x1),
                                _mm_mul_ps(val0125, x2),
                                _mm_mul_ps(val0125, y1),
                                _mm_mul_ps(val0125, y2)),
                            _mm_mul_ps(
                                val05,
                                noise.Evaluate_SIMD(
                                    _mm_mul_ps(val025, x1),
                                    _mm_mul_ps(val025, x2),
                                    _mm_mul_ps(val025, y1),
                                    _mm_mul_ps(val025, y2)))),
                        _mm_add_ps(
                            _mm_mul_ps(
                                val025,
                                noise.Evaluate_SIMD(
                                    _mm_mul_ps(val05, x1),
                                    _mm_mul_ps(val05, x2),
                                    _mm_mul_ps(val05, y1),
                                    _mm_mul_ps(val05, y2))),
                            _mm_mul_ps(val0125, fineNoiseSampls))));
            }
        });

    noiseWidth = width;
    noiseHeight = height;

    computeBlurNoise();
}

void Random::computeBlurNoise()
{
    std::cout << "Blurring noise\n";
    blurNoise.resize(coarseNoise.size());
    // Fast approximate Gaussian blur via horizontal/vertical smearing with
    // rolling averages.
    parallelFor(std::views::iota(0, noiseWidth), [this](int x) {
        double accu = 0;
        for (int y = noiseHeight - 40; y < noiseHeight; ++y) {
            accu = 0.9 * accu + 0.1 * coarseNoise[x * noiseHeight + y];
        }
        for (int y = 0; y < noiseHeight; ++y) {
            accu = 0.9 * accu + 0.1 * coarseNoise[x * noiseHeight + y];
            blurNoise[x * noiseHeight + y] = accu;
        }
    });
    parallelFor(std::views::iota(0, noiseHeight), [this](int y) {
        double accu = 0;
        for (int x = noiseWidth - 40; x < noiseWidth; ++x) {
            accu = 0.9 * accu + 0.1 * blurNoise[x * noiseHeight + y];
        }
        for (int x = 0; x < noiseWidth; ++x) {
            accu = 0.9 * accu + 0.1 * blurNoise[x * noiseHeight + y];
            blurNoise[x * noiseHeight + y] = accu;
        }
    });
}

int Random::getPoolIndex(int size, std::source_location origin)
{
    std::string key = std::to_string(origin.line()) + ':' +
                      std::to_string(origin.column()) + ':' +
                      origin.function_name();
    if (!poolState.contains(key)) {
        poolState[key] = getInt(0, size - 1);
    }
    ++poolState[key];
    return poolState[key] % size;
}

void Random::shuffleNoise()
{
    noiseDeltaX = getInt(0, noiseWidth);
    noiseDeltaY = getInt(0, noiseHeight);
}

void Random::saveShuffleState()
{
    savedNoiseDeltaX = noiseDeltaX;
    savedNoiseDeltaY = noiseDeltaY;
}

void Random::restoreShuffleState()
{
    noiseDeltaX = savedNoiseDeltaX;
    noiseDeltaY = savedNoiseDeltaY;
}

bool Random::getBool()
{
    return getInt(0, 1) == 0;
}

uint8_t Random::getByte()
{
    return getInt(0, 0xff);
}

double Random::getDouble(double min, double max)
{
    std::uniform_real_distribution<> dist(min, max);
    return dist(rnd);
}

int Random::getInt(int min, int max)
{
    std::uniform_int_distribution<> dist(min, max);
    return dist(rnd);
}

double Random::getBlurNoise(int x, int y) const
{
    // Note: positive out-of-bounds is fine, negative may crash.
    return blurNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
}

double Random::getCoarseNoise(int x, int y) const
{
    // Note: positive out-of-bounds is fine, negative may crash.
    return coarseNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
}

double Random::getFineNoise(int x, int y) const
{
    // Note: positive out-of-bounds is fine, negative may crash.
    return fineNoise
        [noiseHeight * ((x + noiseDeltaX) % noiseWidth) +
         ((y + noiseDeltaY) % noiseHeight)];
}

std::vector<int> Random::partitionRange(int numSegments, int range)
{
    std::uniform_real_distribution<> dist(0.1, 1.0);
    double total = 0;
    std::vector<double> segments;
    for (int i = 0; i < numSegments; ++i) {
        total += dist(rnd);
        segments.push_back(total);
    }
    segments.pop_back();
    std::vector<int> partitions;
    for (double seg : segments) {
        partitions.push_back(range * (seg / total));
    }
    return partitions;
}
