#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cstdint>
#include <cmath>

using namespace emscripten;

/**
 * 흑백(Grayscale) 필터 구현 - 고성능 버전
 * WASM 메모리 직접 접근으로 JS↔WASM 경계 호출 최소화
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param length 데이터 길이 (바이트)
 */
void applyGrayscale(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // 순수 C++ 연산 - JS 호출 없음
    for (int i = 0; i < length; i += 4) {
        // ITU-R BT.709 표준 공식
        // 정수 연산으로 최적화 (소수점 연산 회피)
        // Y = (54*R + 183*G + 19*B) >> 8  (≈ 0.2126*R + 0.7152*G + 0.0722*B)
        uint8_t gray = static_cast<uint8_t>(
            (54 * data[i] + 183 * data[i + 1] + 19 * data[i + 2]) >> 8
        );

        data[i] = gray;
        data[i + 1] = gray;
        data[i + 2] = gray;
        // Alpha (data[i + 3])는 그대로 유지
    }
}

/**
 * 좌우반전(Horizontal Flip) 필터 구현 - 고성능 버전
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 */
void applyHorizontalFlip(uintptr_t dataPtr, int width, int height) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // 각 행에서 좌우 픽셀을 교환
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width / 2; x++) {
            int leftIdx = (y * width + x) * 4;
            int rightIdx = (y * width + (width - 1 - x)) * 4;

            // RGBA 4바이트를 한번에 교환
            for (int c = 0; c < 4; c++) {
                uint8_t temp = data[leftIdx + c];
                data[leftIdx + c] = data[rightIdx + c];
                data[rightIdx + c] = temp;
            }
        }
    }
}

/**
 * 세피아(Sepia) 필터 구현 - 고성능 버전
 * 빈티지/레트로 사진 효과
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param length 데이터 길이 (바이트)
 */
void applySepia(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int i = 0; i < length; i += 4) {
        uint8_t r = data[i];
        uint8_t g = data[i + 1];
        uint8_t b = data[i + 2];

        // 세피아 변환 공식 (정수 연산 최적화)
        // newR = (r * 0.393 + g * 0.769 + b * 0.189)
        // newG = (r * 0.349 + g * 0.686 + b * 0.168)
        // newB = (r * 0.272 + g * 0.534 + b * 0.131)
        int newR = (r * 101 + g * 197 + b * 48) >> 8;
        int newG = (r * 89 + g * 176 + b * 43) >> 8;
        int newB = (r * 70 + g * 137 + b * 34) >> 8;

        // 255 클램핑
        data[i] = static_cast<uint8_t>(newR > 255 ? 255 : newR);
        data[i + 1] = static_cast<uint8_t>(newG > 255 ? 255 : newG);
        data[i + 2] = static_cast<uint8_t>(newB > 255 ? 255 : newB);
        // Alpha (data[i + 3])는 그대로 유지
    }
}

/**
 * 픽셀아트(Pixelate) 필터 구현 - 고성능 버전
 * 레트로 게임 스타일로 변환 (해상도 낮추기 + 팔레트 제한)
 *
 * 최적화:
 * - 나눗셈을 역수 곱셈으로 대체 (/ count → * invCount >> 16)
 * - 곱셈 인덱스 계산을 비트 시프트로 대체
 * - 블록 크기별 역수 테이블 사용
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param blockSize 픽셀 블록 크기 (클수록 더 픽셀화)
 */
void applyPixelate(uintptr_t dataPtr, int width, int height, int blockSize) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // 최소 블록 크기 보장
    if (blockSize < 2) blockSize = 2;
    if (blockSize > 32) blockSize = 32;  // 최대 제한

    // 역수 테이블 (1~100 범위, 16비트 고정소수점)
    // invTable[n] = (65536 / n) → x / n ≈ (x * invTable[n]) >> 16
    // blockSize 최대 32이므로 count 최대 = 32*32 = 1024
    // 하지만 실제로 100 이하만 테이블 사용, 나머지는 런타임 계산
    static const uint16_t invTable[101] = {
        0, 65535, 32768, 21845, 16384, 13107, 10923, 9362, 8192, 7282, 6554,  // 0-10
        5958, 5461, 5041, 4681, 4369, 4096, 3855, 3641, 3449, 3277,           // 11-20
        3121, 2979, 2849, 2731, 2621, 2521, 2427, 2341, 2260, 2185,           // 21-30
        2114, 2048, 1986, 1928, 1872, 1820, 1771, 1725, 1680, 1638,           // 31-40
        1598, 1560, 1524, 1489, 1456, 1425, 1394, 1365, 1337, 1311,           // 41-50
        1285, 1260, 1237, 1214, 1192, 1170, 1150, 1130, 1111, 1092,           // 51-60
        1074, 1057, 1040, 1024, 1008, 993, 978, 964, 950, 936,                // 61-70
        923, 910, 898, 886, 874, 862, 851, 840, 830, 819,                     // 71-80
        809, 799, 790, 780, 771, 762, 753, 745, 736, 728,                     // 81-90
        720, 712, 705, 697, 690, 683, 676, 669, 662, 655                      // 91-100
    };

    for (int y = 0; y < height; y += blockSize) {
        for (int x = 0; x < width; x += blockSize) {
            // 블록 내 평균 색상 계산
            int sumR = 0, sumG = 0, sumB = 0;

            int blockH = (y + blockSize > height) ? height - y : blockSize;
            int blockW = (x + blockSize > width) ? width - x : blockSize;
            int count = blockH * blockW;

            // 합계 계산
            for (int by = 0; by < blockH; by++) {
                int rowOffset = (y + by) * width * 4;
                for (int bx = 0; bx < blockW; bx++) {
                    int idx = rowOffset + ((x + bx) << 2);
                    sumR += data[idx];
                    sumG += data[idx + 1];
                    sumB += data[idx + 2];
                }
            }

            // 평균 계산 (나눗셈 대신 역수 곱셈)
            uint16_t inv = (count <= 100) ? invTable[count] : (65536 / count);
            uint8_t avgR = static_cast<uint8_t>(((sumR * inv) >> 16) & 0xF8);
            uint8_t avgG = static_cast<uint8_t>(((sumG * inv) >> 16) & 0xF8);
            uint8_t avgB = static_cast<uint8_t>(((sumB * inv) >> 16) & 0xF8);

            // 블록 전체에 평균 색상 적용
            for (int by = 0; by < blockH; by++) {
                int rowOffset = (y + by) * width * 4;
                for (int bx = 0; bx < blockW; bx++) {
                    int idx = rowOffset + ((x + bx) << 2);
                    data[idx] = avgR;
                    data[idx + 1] = avgG;
                    data[idx + 2] = avgB;
                }
            }
        }
    }
}

/**
 * 글리치(Glitch) 효과 필터 구현 - 고성능 버전
 * RGB 채널 분리 + 스캔라인 + 색상 왜곡 (사이버펑크 느낌)
 *
 * 최적화:
 * - 나눗셈을 비트 시프트로 대체 (* 85 / 100 → * 217 >> 8 ≈ 0.848)
 * - 나눗셈 2를 >> 1로 대체
 * - 불필요한 경계 체크 최소화
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param intensity 효과 강도 (0-100)
 * @param seed 랜덤 시드 (애니메이션용)
 */
void applyGlitch(uintptr_t dataPtr, int width, int height, int intensity, int seed) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // 간단한 PRNG (인라인 가능)
    auto rand = [&seed]() {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        return seed;
    };

    int shift = 3 + (intensity >> 5);  // intensity / 32 ≈ intensity / 25
    if (shift >= (width >> 2)) shift = width >> 2;

    // 1. 스캔라인 효과 (짝수 라인 어둡게)
    // * 85 / 100 ≈ * 217 >> 8 (0.848 vs 0.85, 오차 0.2%)
    for (int y = 0; y < height; y += 2) {
        int rowOffset = y * width * 4;
        for (int x = 0; x < width; x++) {
            int idx = rowOffset + (x << 2);  // x * 4
            data[idx] = static_cast<uint8_t>((data[idx] * 217) >> 8);
            data[idx + 1] = static_cast<uint8_t>((data[idx + 1] * 217) >> 8);
            data[idx + 2] = static_cast<uint8_t>((data[idx + 2] * 217) >> 8);
        }
    }

    // 2. RGB 채널 시프트 (간단한 색수차 효과)
    for (int y = 0; y < height; y++) {
        int rowOffset = y * width * 4;
        for (int x = shift; x < width - shift; x++) {
            int idx = rowOffset + (x << 2);
            int leftIdx = rowOffset + ((x - shift) << 2);
            int rightIdx = rowOffset + ((x + shift) << 2);

            // / 2 → >> 1
            data[idx] = static_cast<uint8_t>((data[idx] + data[leftIdx]) >> 1);
            data[idx + 2] = static_cast<uint8_t>((data[idx + 2] + data[rightIdx + 2]) >> 1);
        }
    }

    // 3. 랜덤 글리치 라인 (일부 라인 색상 반전)
    int numGlitchLines = intensity >> 5;  // / 32 ≈ / 25
    for (int i = 0; i < numGlitchLines && i < 5; i++) {
        int y = rand() % height;
        int startX = rand() % (width >> 1);  // / 2
        int lineWidth = 30 + (rand() & 0x7F);  // % 128 대신 & 0x7F (근사값)
        int endX = startX + lineWidth;
        if (endX > width) endX = width;

        int rowOffset = y * width * 4;
        for (int x = startX; x < endX; x++) {
            int idx = rowOffset + (x << 2);
            // XOR로 색상 반전 (255 - x == x ^ 255)
            data[idx] ^= 255;
            data[idx + 1] ^= 255;
            data[idx + 2] ^= 255;
        }
    }

    // 4. 노이즈 블록 (마젠타/시안)
    int numBlocks = intensity / 30;
    for (int i = 0; i < numBlocks && i < 4; i++) {
        int maxBlockW = 40;
        int maxBlockH = 4;

        if (width <= maxBlockW || height <= maxBlockH) continue;

        int blockX = rand() % (width - maxBlockW);
        int blockY = rand() % (height - maxBlockH);
        int blockW = 15 + (rand() & 0x1F);  // % 32 대신 & 0x1F
        int blockH = 2 + (rand() & 0x1);    // % 2 대신 & 0x1

        // 경계 클램핑
        if (blockX + blockW > width) blockW = width - blockX;
        if (blockY + blockH > height) blockH = height - blockY;

        uint8_t r = (rand() & 1) ? 255 : 0;  // % 2 대신 & 1
        uint8_t g = 0;
        uint8_t b = 255;

        for (int by = 0; by < blockH; by++) {
            int rowOffset = (blockY + by) * width * 4;
            for (int bx = 0; bx < blockW; bx++) {
                int idx = rowOffset + ((blockX + bx) << 2);
                data[idx] = r;
                data[idx + 1] = g;
                data[idx + 2] = b;
            }
        }
    }
}

// 열화상 LUT (Lookup Table) - 컴파일 타임 생성
// 256개 밝기 값에 대한 RGB 값을 미리 계산
namespace {
    struct ThermalLUT {
        uint8_t r[256];
        uint8_t g[256];
        uint8_t b[256];

        constexpr ThermalLUT() : r{}, g{}, b{} {
            for (int i = 0; i < 256; i++) {
                if (i < 43) {
                    // 검정 → 파랑: (0,0,0) → (0,0,255)
                    r[i] = 0;
                    g[i] = 0;
                    b[i] = (i * 255) / 43;
                } else if (i < 85) {
                    // 파랑 → 시안: (0,0,255) → (0,255,255)
                    r[i] = 0;
                    g[i] = ((i - 43) * 255) / 42;
                    b[i] = 255;
                } else if (i < 128) {
                    // 시안 → 녹색: (0,255,255) → (0,255,0)
                    r[i] = 0;
                    g[i] = 255;
                    b[i] = 255 - ((i - 85) * 255) / 43;
                } else if (i < 170) {
                    // 녹색 → 노랑: (0,255,0) → (255,255,0)
                    r[i] = ((i - 128) * 255) / 42;
                    g[i] = 255;
                    b[i] = 0;
                } else if (i < 213) {
                    // 노랑 → 빨강: (255,255,0) → (255,0,0)
                    r[i] = 255;
                    g[i] = 255 - ((i - 170) * 255) / 43;
                    b[i] = 0;
                } else {
                    // 빨강 → 흰색: (255,0,0) → (255,255,255)
                    r[i] = 255;
                    g[i] = ((i - 213) * 255) / 42;
                    b[i] = ((i - 213) * 255) / 42;
                }
            }
        }
    };

    constexpr ThermalLUT thermalLUT{};
}

/**
 * 열화상 카메라(Thermal) 필터 구현 - 고성능 LUT 버전
 * 밝기 기반 온도 색상 매핑 (파랑→시안→녹색→노랑→빨강→흰색)
 *
 * 최적화:
 * - LUT(Lookup Table)로 실시간 연산 제거
 * - 분기문 완전 제거 (O(1) 테이블 조회)
 * - float 연산 제거
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param length 데이터 길이 (바이트)
 */
void applyThermal(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int i = 0; i < length; i += 4) {
        // 밝기 계산 (0-255) - 정수 연산
        uint8_t brightness = static_cast<uint8_t>(
            (54 * data[i] + 183 * data[i + 1] + 19 * data[i + 2]) >> 8
        );

        // LUT 조회 - O(1), 분기 없음
        data[i] = thermalLUT.r[brightness];
        data[i + 1] = thermalLUT.g[brightness];
        data[i + 2] = thermalLUT.b[brightness];
        // Alpha (data[i + 3])는 그대로 유지
    }
}

/**
 * WASM 메모리 할당 함수
 * JavaScript에서 데이터를 복사할 버퍼 생성
 *
 * @param size 할당할 바이트 수
 * @return 할당된 메모리 포인터
 */
uintptr_t allocateBuffer(int size) {
    return reinterpret_cast<uintptr_t>(malloc(size));
}

/**
 * WASM 메모리 해제 함수
 *
 * @param ptr 해제할 메모리 포인터
 */
void freeBuffer(uintptr_t ptr) {
    free(reinterpret_cast<void*>(ptr));
}

// JavaScript에 함수 노출
EMSCRIPTEN_BINDINGS(filters) {
    function("applyGrayscale", &applyGrayscale);
    function("applyHorizontalFlip", &applyHorizontalFlip);
    function("applySepia", &applySepia);
    function("applyPixelate", &applyPixelate);
    function("applyGlitch", &applyGlitch);
    function("applyThermal", &applyThermal);
    function("allocateBuffer", &allocateBuffer);
    function("freeBuffer", &freeBuffer);
}
