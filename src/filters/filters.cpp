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
 * 픽셀아트(Pixelate) 필터 구현
 * 레트로 게임 스타일로 변환 (해상도 낮추기 + 팔레트 제한)
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

    for (int y = 0; y < height; y += blockSize) {
        for (int x = 0; x < width; x += blockSize) {
            // 블록 내 평균 색상 계산
            int sumR = 0, sumG = 0, sumB = 0;
            int count = 0;

            int blockH = (y + blockSize > height) ? height - y : blockSize;
            int blockW = (x + blockSize > width) ? width - x : blockSize;

            for (int by = 0; by < blockH; by++) {
                for (int bx = 0; bx < blockW; bx++) {
                    int idx = ((y + by) * width + (x + bx)) * 4;
                    sumR += data[idx];
                    sumG += data[idx + 1];
                    sumB += data[idx + 2];
                    count++;
                }
            }

            // 평균 색상 (팔레트 제한: 32단계로 양자화)
            uint8_t avgR = static_cast<uint8_t>((sumR / count) & 0xF8);
            uint8_t avgG = static_cast<uint8_t>((sumG / count) & 0xF8);
            uint8_t avgB = static_cast<uint8_t>((sumB / count) & 0xF8);

            // 블록 전체에 평균 색상 적용
            for (int by = 0; by < blockH; by++) {
                for (int bx = 0; bx < blockW; bx++) {
                    int idx = ((y + by) * width + (x + bx)) * 4;
                    data[idx] = avgR;
                    data[idx + 1] = avgG;
                    data[idx + 2] = avgB;
                }
            }
        }
    }
}

/**
 * 글리치(Glitch) 효과 필터 구현
 * RGB 채널 분리 + 스캔라인 + 색상 왜곡 (사이버펑크 느낌)
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param intensity 효과 강도 (0-100)
 * @param seed 랜덤 시드 (애니메이션용)
 */
void applyGlitch(uintptr_t dataPtr, int width, int height, int intensity, int seed) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // 간단한 PRNG
    auto rand = [&seed]() {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        return seed;
    };

    int shift = 3 + (intensity / 25);  // RGB 채널 분리 정도 (3~7 픽셀)
    if (shift >= width / 4) shift = width / 4;

    // 1. 스캔라인 효과 (짝수 라인 어둡게)
    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;
            data[idx] = static_cast<uint8_t>(data[idx] * 85 / 100);
            data[idx + 1] = static_cast<uint8_t>(data[idx + 1] * 85 / 100);
            data[idx + 2] = static_cast<uint8_t>(data[idx + 2] * 85 / 100);
        }
    }

    // 2. RGB 채널 시프트 (간단한 색수차 효과)
    for (int y = 0; y < height; y++) {
        for (int x = shift; x < width - shift; x++) {
            int idx = (y * width + x) * 4;
            int leftIdx = (y * width + (x - shift)) * 4;
            int rightIdx = (y * width + (x + shift)) * 4;

            // Red는 왼쪽 픽셀에서, Blue는 오른쪽 픽셀에서 가져와 혼합
            data[idx] = static_cast<uint8_t>((data[idx] + data[leftIdx]) / 2);
            data[idx + 2] = static_cast<uint8_t>((data[idx + 2] + data[rightIdx + 2]) / 2);
        }
    }

    // 3. 랜덤 글리치 라인 (일부 라인 색상 반전)
    int numGlitchLines = intensity / 25;
    for (int i = 0; i < numGlitchLines && i < 5; i++) {
        int y = rand() % height;
        int startX = rand() % (width / 2);
        int lineWidth = 30 + rand() % 100;
        int endX = startX + lineWidth;
        if (endX > width) endX = width;

        for (int x = startX; x < endX; x++) {
            int idx = (y * width + x) * 4;
            // 색상 반전
            data[idx] = 255 - data[idx];
            data[idx + 1] = 255 - data[idx + 1];
            data[idx + 2] = 255 - data[idx + 2];
        }
    }

    // 4. 노이즈 블록 (마젠타/시안) - 경계 체크 강화
    int numBlocks = intensity / 30;
    for (int i = 0; i < numBlocks && i < 4; i++) {
        int maxBlockW = 40;
        int maxBlockH = 4;

        if (width <= maxBlockW || height <= maxBlockH) continue;

        int blockX = rand() % (width - maxBlockW);
        int blockY = rand() % (height - maxBlockH);
        int blockW = 15 + rand() % 25;
        int blockH = 2 + rand() % 2;

        // 경계 재확인
        if (blockX + blockW > width) blockW = width - blockX;
        if (blockY + blockH > height) blockH = height - blockY;

        uint8_t r = (rand() % 2 == 0) ? 255 : 0;
        uint8_t g = 0;
        uint8_t b = 255;

        for (int by = 0; by < blockH; by++) {
            for (int bx = 0; bx < blockW; bx++) {
                int px = blockX + bx;
                int py = blockY + by;
                if (px >= 0 && px < width && py >= 0 && py < height) {
                    int idx = (py * width + px) * 4;
                    data[idx] = r;
                    data[idx + 1] = g;
                    data[idx + 2] = b;
                }
            }
        }
    }
}

/**
 * 열화상 카메라(Thermal) 필터 구현
 * 밝기 기반 온도 색상 매핑 (파랑→시안→녹색→노랑→빨강→흰색)
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param length 데이터 길이 (바이트)
 */
void applyThermal(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int i = 0; i < length; i += 4) {
        // 밝기 계산 (0-255)
        int brightness = (data[i] * 54 + data[i + 1] * 183 + data[i + 2] * 19) >> 8;

        uint8_t r, g, b;

        // 온도 색상 매핑 (6단계 그라데이션)
        if (brightness < 43) {
            // 검정 → 파랑
            float t = brightness / 43.0f;
            r = 0;
            g = 0;
            b = static_cast<uint8_t>(t * 255);
        } else if (brightness < 85) {
            // 파랑 → 시안
            float t = (brightness - 43) / 42.0f;
            r = 0;
            g = static_cast<uint8_t>(t * 255);
            b = 255;
        } else if (brightness < 128) {
            // 시안 → 녹색
            float t = (brightness - 85) / 43.0f;
            r = 0;
            g = 255;
            b = static_cast<uint8_t>(255 * (1 - t));
        } else if (brightness < 170) {
            // 녹색 → 노랑
            float t = (brightness - 128) / 42.0f;
            r = static_cast<uint8_t>(t * 255);
            g = 255;
            b = 0;
        } else if (brightness < 213) {
            // 노랑 → 빨강
            float t = (brightness - 170) / 43.0f;
            r = 255;
            g = static_cast<uint8_t>(255 * (1 - t));
            b = 0;
        } else {
            // 빨강 → 흰색
            float t = (brightness - 213) / 42.0f;
            r = 255;
            g = static_cast<uint8_t>(t * 255);
            b = static_cast<uint8_t>(t * 255);
        }

        data[i] = r;
        data[i + 1] = g;
        data[i + 2] = b;
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
