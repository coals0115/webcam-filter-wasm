#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cstdint>

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

// JavaScript에서 호출 가능하도록 함수 바인딩
EMSCRIPTEN_BINDINGS(webcam_filter) {
    function("applyGrayscaleFilterRaw", &applyGrayscaleFilterRaw);
}
