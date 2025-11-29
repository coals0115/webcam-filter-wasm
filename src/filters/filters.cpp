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
    function("allocateBuffer", &allocateBuffer);
    function("freeBuffer", &freeBuffer);
}
