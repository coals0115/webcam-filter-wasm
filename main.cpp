#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cstdint>
#include <algorithm>

using namespace emscripten;

/**
 * 흑백 필터 함수
 * @param ptr 메모리 포인터 (uintptr_t로 받음)
 * @param length 배열 길이
 */
void applyGrayscaleFilterRaw(uintptr_t ptr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);

    for (int i = 0; i < length; i += 4) {
        uint8_t r = data[i];
        uint8_t g = data[i + 1];
        uint8_t b = data[i + 2];

        // 그레이스케일 변환 (ITU-R BT.709)
        uint8_t gray = static_cast<uint8_t>(
            0.2126 * r + 0.7152 * g + 0.0722 * b
        );

        data[i] = gray;
        data[i + 1] = gray;
        data[i + 2] = gray;
        // data[i + 3]는 alpha, 그대로 유지
    }
}

/**
 * 좌우반전 필터 함수
 * @param ptr 메모리 포인터 (uintptr_t로 받음)
 * @param width 이미지 너비
 * @param height 이미지 높이
 */
void applyHorizontalFlipRaw(uintptr_t ptr, int width, int height) {
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width / 2; x++) {
            int leftIndex = (y * width + x) * 4;
            int rightIndex = (y * width + (width - 1 - x)) * 4;

            // 좌우 픽셀 교환 (R, G, B, A)
            for (int c = 0; c < 4; c++) {
                uint8_t temp = data[leftIndex + c];
                data[leftIndex + c] = data[rightIndex + c];
                data[rightIndex + c] = temp;
            }
        }
    }
}

/**
 * 세피아 필터 함수
 * @param ptr 메모리 포인터
 * @param length 배열 길이
 */
void applySepiaFilterRaw(uintptr_t ptr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);

    for (int i = 0; i < length; i += 4) {
        // 1. 원본 RGB 값 읽기
        uint8_t r = data[i];
        uint8_t g = data[i+1];
        uint8_t b = data[i+2];

        // 2. 세피아 변환 공식 적용
        uint8_t tr = static_cast<uint8_t>(
            std::min(255.0, (r * 0.393 + g * 0.769 + b * 0.189))
        );
        uint8_t tg = static_cast<uint8_t>(
            std::min(255.0, (r * 0.349 + g * 0.686 + b * 0.168))
        );
        uint8_t tb = static_cast<uint8_t>(
            std::min(255.0, (r * 0.272 + g * 0.534 + b * 0.131))
        );

        // 3. 변환된 값 저장
        data[i] = tr;
        data[i+1] = tg;
        data[i+2] = tb;
    }
}

// JavaScript에서 호출 가능하도록 함수 바인딩
EMSCRIPTEN_BINDINGS(webcam_filter) {
    function("applyGrayscaleFilterRaw", &applyGrayscaleFilterRaw);
    function("applyHorizontalFlipRaw", &applyHorizontalFlipRaw);
    function("applySepiaFilterRaw", &applySepiaFilterRaw);
}