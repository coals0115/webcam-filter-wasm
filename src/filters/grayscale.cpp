#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cstdint>
#include <cmath>

using namespace emscripten;

/**
 * 흑백(Grayscale) 필터 구현
 * ITU-R BT.709 표준 사용
 *
 * @param imageData JavaScript ImageData 객체 (RGBA 형식)
 * @return void (원본 데이터를 직접 수정)
 */
void applyGrayscale(val imageData) {
    // ImageData에서 픽셀 데이터 추출
    val data = imageData["data"];
    unsigned int length = data["length"].as<unsigned int>();

    // 각 픽셀 처리 (RGBA 형식: 4바이트씩)
    for (unsigned int i = 0; i < length; i += 4) {
        // RGB 값 읽기
        uint8_t r = data[i].as<uint8_t>();
        uint8_t g = data[i + 1].as<uint8_t>();
        uint8_t b = data[i + 2].as<uint8_t>();
        // Alpha는 i + 3

        // ITU-R BT.709 표준 공식
        // Y = 0.2126 * R + 0.7152 * G + 0.0722 * B
        // 인간의 눈이 녹색에 가장 민감하므로 가중치가 다름
        uint8_t gray = static_cast<uint8_t>(
            0.2126 * r + 0.7152 * g + 0.0722 * b
        );

        // RGB 값을 동일한 gray 값으로 설정
        data.call<void>("set", i, gray);
        data.call<void>("set", i + 1, gray);
        data.call<void>("set", i + 2, gray);
        // Alpha는 그대로 유지
    }
}

// JavaScript에 함수 노출
EMSCRIPTEN_BINDINGS(filters) {
    function("applyGrayscale", &applyGrayscale);
}
