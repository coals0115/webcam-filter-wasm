#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cstdint>
#include <cmath>

using namespace emscripten;

/**
 * 흑백(Grayscale) 필터 구현 - 고성능 버전
 * ITU-R BT.601 표준 기반 밝기 변환
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param length 데이터 길이 (바이트)
 */
void applyGrayscale(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int i = 0; i < length; i += 4) {
        uint8_t r = data[i];
        uint8_t g = data[i + 1];
        uint8_t b = data[i + 2];

        // ITU-R BT.601 표준 (정수 연산 최적화)
        // gray = 0.299*R + 0.587*G + 0.114*B
        // gray = (R*77 + G*150 + B*29) >> 8
        uint8_t gray = static_cast<uint8_t>((r * 77 + g * 150 + b * 29) >> 8);

        data[i] = gray;
        data[i + 1] = gray;
        data[i + 2] = gray;
        // Alpha (data[i + 3])는 그대로 유지
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
 * 열화상 카메라(Thermal) 필터 구현 - 정수 연산 최적화 버전
 * 밝기 기반 온도 색상 매핑 (파랑→시안→녹색→노랑→빨강→흰색)
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param length 데이터 길이 (바이트)
 */
void applyThermal(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int i = 0; i < length; i += 4) {
        // 밝기 계산 (0-255) - 정수 연산
        int brightness = (data[i] * 54 + data[i + 1] * 183 + data[i + 2] * 19) >> 8;

        uint8_t r, g, b;

        // 온도 색상 매핑 (6단계 그라데이션) - 정수 연산 최적화
        // float 나눗셈 제거: (val * 255) / range 형태로 변환
        if (brightness < 43) {
            // 검정 → 파랑: t = brightness / 43, b = t * 255 = (brightness * 255) / 43
            r = 0;
            g = 0;
            b = static_cast<uint8_t>((brightness * 255) / 43);
        } else if (brightness < 85) {
            // 파랑 → 시안: t = (brightness - 43) / 42
            int t = brightness - 43;
            r = 0;
            g = static_cast<uint8_t>((t * 255) / 42);
            b = 255;
        } else if (brightness < 128) {
            // 시안 → 녹색: t = (brightness - 85) / 43
            int t = brightness - 85;
            r = 0;
            g = 255;
            b = static_cast<uint8_t>(255 - (t * 255) / 43);
        } else if (brightness < 170) {
            // 녹색 → 노랑: t = (brightness - 128) / 42
            int t = brightness - 128;
            r = static_cast<uint8_t>((t * 255) / 42);
            g = 255;
            b = 0;
        } else if (brightness < 213) {
            // 노랑 → 빨강: t = (brightness - 170) / 43
            int t = brightness - 170;
            r = 255;
            g = static_cast<uint8_t>(255 - (t * 255) / 43);
            b = 0;
        } else {
            // 빨강 → 흰색: t = (brightness - 213) / 42
            int t = brightness - 213;
            int val = (t * 255) / 42;
            r = 255;
            g = static_cast<uint8_t>(val);
            b = static_cast<uint8_t>(val);
        }

        data[i] = r;
        data[i + 1] = g;
        data[i + 2] = b;
    }
}

/**
 * 거울(Mirror) 효과 필터 구현
 * 좌우 대칭 거울 효과 (왼쪽을 오른쪽에 복사)
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param mode 모드: 0=좌우대칭, 1=상하대칭, 2=4분할
 */
void applyMirror(uintptr_t dataPtr, int width, int height, int mode) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    if (mode == 0) {
        // 좌우 대칭: 왼쪽 절반을 오른쪽에 반전 복사
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width / 2; x++) {
                int srcIdx = (y * width + x) * 4;
                int dstIdx = (y * width + (width - 1 - x)) * 4;

                data[dstIdx] = data[srcIdx];
                data[dstIdx + 1] = data[srcIdx + 1];
                data[dstIdx + 2] = data[srcIdx + 2];
                data[dstIdx + 3] = data[srcIdx + 3];
            }
        }
    } else if (mode == 1) {
        // 상하 대칭: 위쪽 절반을 아래쪽에 반전 복사
        for (int y = 0; y < height / 2; y++) {
            for (int x = 0; x < width; x++) {
                int srcIdx = (y * width + x) * 4;
                int dstIdx = ((height - 1 - y) * width + x) * 4;

                data[dstIdx] = data[srcIdx];
                data[dstIdx + 1] = data[srcIdx + 1];
                data[dstIdx + 2] = data[srcIdx + 2];
                data[dstIdx + 3] = data[srcIdx + 3];
            }
        }
    } else {
        // 4분할 거울: 좌상단을 나머지 3개 영역에 복사
        int halfW = width / 2;
        int halfH = height / 2;

        for (int y = 0; y < halfH; y++) {
            for (int x = 0; x < halfW; x++) {
                int srcIdx = (y * width + x) * 4;

                // 우상단 (좌우 반전)
                int idx1 = (y * width + (width - 1 - x)) * 4;
                // 좌하단 (상하 반전)
                int idx2 = ((height - 1 - y) * width + x) * 4;
                // 우하단 (좌우+상하 반전)
                int idx3 = ((height - 1 - y) * width + (width - 1 - x)) * 4;

                for (int c = 0; c < 4; c++) {
                    data[idx1 + c] = data[srcIdx + c];
                    data[idx2 + c] = data[srcIdx + c];
                    data[idx3 + c] = data[srcIdx + c];
                }
            }
        }
    }
}

/**
 * 크로마키(Chroma Key) 필터 구현 - 고성능 버전
 * 특정 색상을 감지하여 배경 이미지로 대체
 *
 * @param framePtr 전경(웹캠) 이미지 RGBA 버퍼 포인터
 * @param bgPtr 배경 이미지 RGBA 버퍼 포인터
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param keyR 크로마키 기준색 R
 * @param keyG 크로마키 기준색 G
 * @param keyB 크로마키 기준색 B
 * @param tolerance 허용 오차 범위
 */
void applyChromaKey(uintptr_t framePtr, uintptr_t bgPtr, int width, int height,
                    int keyR, int keyG, int keyB, int tolerance) {

    uint8_t* frame = reinterpret_cast<uint8_t*>(framePtr);
    uint8_t* bg = reinterpret_cast<uint8_t*>(bgPtr);

    int total = width * height;
    int tolSq = tolerance * tolerance;

    for (int i = 0; i < total; i++) {
        int idx = i * 4;

        int dr = frame[idx]     - keyR;
        int dg = frame[idx + 1] - keyG;
        int db = frame[idx + 2] - keyB;

        int distSq = dr * dr + dg * dg + db * db;

        if (distSq <= tolSq) {
            frame[idx]     = bg[idx];
            frame[idx + 1] = bg[idx + 1];
            frame[idx + 2] = bg[idx + 2];
            frame[idx + 3] = bg[idx + 3];
        }
    }
}

/**
 * X-Ray 필터 구현
 * 명암 반전(Negative) 및 대비 증강 효과 적용
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param length 데이터 길이 (바이트)
 */
void applyXrayFilter(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // 대비 조절을 위한 상수. 값이 클수록 대비가 강해짐.
    // 0 ~ 255 범위의 픽셀 값을 조정. 128을 중심으로 픽셀 값을 조정.
    const int contrast_factor = 80;

    for (int i = 0; i < length; i += 4) {

        // 1. 명암 반전
        int r = 255 - data[i];
        int g = 255 - data[i + 1];
        int b = 255 - data[i + 2];

        // 2. 대비 증강
        int newR = 128 + ((r - 128) * contrast_factor) / 128;
        int newG = 128 + ((g - 128) * contrast_factor) / 128;
        int newB = 128 + ((b - 128) * contrast_factor) / 128;

        // 3. 0-255 클램핑
        // 결과가 0 미만이거나 255 초과하지 않도록 보장
        data[i] = static_cast<uint8_t>(std::max(0, std::min(255, newR)));
        data[i + 1] = static_cast<uint8_t>(std::max(0, std::min(255, newG)));
        data[i + 2] = static_cast<uint8_t>(std::max(0, std::min(255, newB)));

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
    function("applySepia", &applySepia);
    function("applyThermal", &applyThermal);
    function("applyMirror", &applyMirror);
    function("applyChromaKey", &applyChromaKey);
    function("applyXrayFilter", &applyXrayFilter);
    function("allocateBuffer", &allocateBuffer);
    function("freeBuffer", &freeBuffer);
}