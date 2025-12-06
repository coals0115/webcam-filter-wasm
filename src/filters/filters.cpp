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
 * 야간 투시경(Night Vision) 필터 구현
 * 녹색 톤 + 노이즈 + 비네팅 효과 (밀리터리 느낌)
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param seed 노이즈용 랜덤 시드
 */
void applyNightVision(uintptr_t dataPtr, int width, int height, int seed) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // 간단한 PRNG
    auto rand = [&seed]() {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        return seed;
    };

    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;

            // 밝기 계산
            int brightness = (data[idx] * 54 + data[idx + 1] * 183 + data[idx + 2] * 19) >> 8;

            // 밝기 증폭 (야간 투시경은 빛을 증폭)
            brightness = brightness * 150 / 100;
            if (brightness > 255) brightness = 255;

            // 노이즈 추가 (약간의 랜덤 변동)
            int noise = (rand() % 30) - 15;
            brightness += noise;
            if (brightness < 0) brightness = 0;
            if (brightness > 255) brightness = 255;

            // 비네팅 효과 (가장자리 어둡게)
            float dx = x - centerX;
            float dy = y - centerY;
            float dist = std::sqrt(dx * dx + dy * dy);
            float vignette = 1.0f - (dist / maxDist) * 0.6f;

            brightness = static_cast<int>(brightness * vignette);

            // 녹색 톤 적용 (R과 B는 낮게, G는 밝기 그대로)
            data[idx] = static_cast<uint8_t>(brightness * 20 / 100);      // R: 약간
            data[idx + 1] = static_cast<uint8_t>(brightness);              // G: 풀
            data[idx + 2] = static_cast<uint8_t>(brightness * 20 / 100);  // B: 약간
        }
    }
}

/**
 * 만화/애니(Toon/Cartoon) 효과 필터 구현
 * 엣지 검출 + 색상 단순화 (Posterization)
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param levels 색상 단계 수 (낮을수록 더 만화 느낌)
 */
void applyToon(uintptr_t dataPtr, int width, int height, int levels) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    if (levels < 2) levels = 2;
    if (levels > 8) levels = 8;

    int step = 256 / levels;

    // 1단계: 색상 단순화 (Posterization)
    for (int i = 0; i < width * height * 4; i += 4) {
        data[i] = static_cast<uint8_t>((data[i] / step) * step);
        data[i + 1] = static_cast<uint8_t>((data[i + 1] / step) * step);
        data[i + 2] = static_cast<uint8_t>((data[i + 2] / step) * step);
    }

    // 2단계: 간단한 엣지 검출 (Sobel-like)
    // 임시 버퍼에 엣지 정보 저장
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int idx = (y * width + x) * 4;

            // 주변 픽셀과의 밝기 차이 계산
            int centerBright = (data[idx] + data[idx + 1] + data[idx + 2]) / 3;

            int leftIdx = (y * width + (x - 1)) * 4;
            int rightIdx = (y * width + (x + 1)) * 4;
            int topIdx = ((y - 1) * width + x) * 4;
            int bottomIdx = ((y + 1) * width + x) * 4;

            int leftBright = (data[leftIdx] + data[leftIdx + 1] + data[leftIdx + 2]) / 3;
            int rightBright = (data[rightIdx] + data[rightIdx + 1] + data[rightIdx + 2]) / 3;
            int topBright = (data[topIdx] + data[topIdx + 1] + data[topIdx + 2]) / 3;
            int bottomBright = (data[bottomIdx] + data[bottomIdx + 1] + data[bottomIdx + 2]) / 3;

            // 그래디언트 계산
            int gx = rightBright - leftBright;
            int gy = bottomBright - topBright;
            int edge = std::abs(gx) + std::abs(gy);

            // 엣지가 강하면 검은색 윤곽선
            if (edge > 30) {
                data[idx] = static_cast<uint8_t>(data[idx] * 30 / 100);
                data[idx + 1] = static_cast<uint8_t>(data[idx + 1] * 30 / 100);
                data[idx + 2] = static_cast<uint8_t>(data[idx + 2] * 30 / 100);
            }
        }
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
 * Old TV (CRT) 효과 필터 구현
 * 스캔라인 + 색상 번짐 + 약간의 왜곡
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param seed 애니메이션용 시드
 */
void applyOldTV(uintptr_t dataPtr, int width, int height, int seed) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // PRNG
    auto rand = [&seed]() {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        return seed;
    };

    // 1. 스캔라인 효과 (3줄마다 어둡게)
    for (int y = 0; y < height; y++) {
        float scanline = (y % 3 == 0) ? 0.7f : 1.0f;

        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;

            data[idx] = static_cast<uint8_t>(data[idx] * scanline);
            data[idx + 1] = static_cast<uint8_t>(data[idx + 1] * scanline);
            data[idx + 2] = static_cast<uint8_t>(data[idx + 2] * scanline);
        }
    }

    // 2. RGB 색수차 (채널별 약간 시프트)
    int shift = 2;
    for (int y = 0; y < height; y++) {
        for (int x = shift; x < width - shift; x++) {
            int idx = (y * width + x) * 4;
            int leftIdx = (y * width + (x - shift)) * 4;
            int rightIdx = (y * width + (x + shift)) * 4;

            // R은 왼쪽에서, B는 오른쪽에서
            uint8_t newR = data[leftIdx];
            uint8_t newB = data[rightIdx + 2];

            data[idx] = newR;
            data[idx + 2] = newB;
        }
    }

    // 3. 랜덤 수평 노이즈 라인
    int numLines = 3 + rand() % 3;
    for (int i = 0; i < numLines; i++) {
        int y = rand() % height;
        int brightness = 150 + rand() % 50;

        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;
            data[idx] = static_cast<uint8_t>((data[idx] + brightness) / 2);
            data[idx + 1] = static_cast<uint8_t>((data[idx + 1] + brightness) / 2);
            data[idx + 2] = static_cast<uint8_t>((data[idx + 2] + brightness) / 2);
        }
    }

    // 4. 비네팅 (CRT 모니터 가장자리 어두움)
    float centerX = width / 2.0f;
    float centerY = height / 2.0f;
    float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;

            float dx = x - centerX;
            float dy = y - centerY;
            float dist = std::sqrt(dx * dx + dy * dy);
            float vignette = 1.0f - (dist / maxDist) * 0.4f;

            data[idx] = static_cast<uint8_t>(data[idx] * vignette);
            data[idx + 1] = static_cast<uint8_t>(data[idx + 1] * vignette);
            data[idx + 2] = static_cast<uint8_t>(data[idx + 2] * vignette);
        }
    }
}

/**
 * VHS 테이프 효과 필터 구현
 * 트래킹 노이즈 + 색수차 + 워블링
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param seed 애니메이션용 시드
 */
void applyVHS(uintptr_t dataPtr, int width, int height, int seed) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    // PRNG
    auto rand = [&seed]() {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        return seed;
    };

    // 1. 색상 채도 감소 + 약간의 색조 변경 (VHS 특유의 바랜 색감)
    for (int i = 0; i < width * height * 4; i += 4) {
        int r = data[i];
        int g = data[i + 1];
        int b = data[i + 2];

        // 채도 감소
        int gray = (r + g + b) / 3;
        r = (r * 70 + gray * 30) / 100;
        g = (g * 70 + gray * 30) / 100;
        b = (b * 70 + gray * 30) / 100;

        // 약간 빨간/노란 틴트 추가
        r = r + 10 > 255 ? 255 : r + 10;

        data[i] = static_cast<uint8_t>(r);
        data[i + 1] = static_cast<uint8_t>(g);
        data[i + 2] = static_cast<uint8_t>(b);
    }

    // 2. RGB 색수차 (VHS 특유의 색 번짐)
    int shift = 3;
    for (int y = 0; y < height; y++) {
        for (int x = shift; x < width - shift; x++) {
            int idx = (y * width + x) * 4;
            int leftIdx = (y * width + (x - shift)) * 4;
            int rightIdx = (y * width + (x + shift)) * 4;

            data[idx] = (data[idx] + data[leftIdx]) / 2;
            data[idx + 2] = (data[idx + 2] + data[rightIdx + 2]) / 2;
        }
    }

    // 3. 트래킹 노이즈 (화면 일부에 수평 노이즈 밴드)
    int bandY = (seed / 100) % height;
    int bandHeight = 10 + rand() % 20;

    for (int y = bandY; y < bandY + bandHeight && y < height; y++) {
        int offset = (rand() % 10) - 5;  // 수평 흔들림

        for (int x = 0; x < width; x++) {
            int srcX = x + offset;
            if (srcX < 0) srcX = 0;
            if (srcX >= width) srcX = width - 1;

            int dstIdx = (y * width + x) * 4;
            int srcIdx = (y * width + srcX) * 4;

            // 노이즈 추가
            int noise = rand() % 50;
            data[dstIdx] = static_cast<uint8_t>((data[srcIdx] + noise) > 255 ? 255 : data[srcIdx] + noise);
            data[dstIdx + 1] = static_cast<uint8_t>((data[srcIdx + 1] + noise) > 255 ? 255 : data[srcIdx + 1] + noise);
            data[dstIdx + 2] = static_cast<uint8_t>((data[srcIdx + 2] + noise) > 255 ? 255 : data[srcIdx + 2] + noise);
        }
    }

    // 4. 스캔라인 효과
    for (int y = 0; y < height; y += 2) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;
            data[idx] = data[idx] * 90 / 100;
            data[idx + 1] = data[idx + 1] * 90 / 100;
            data[idx + 2] = data[idx + 2] * 90 / 100;
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
    function("applyHorizontalFlip", &applyHorizontalFlip);
    function("applySepia", &applySepia);
    function("applyPixelate", &applyPixelate);
    function("applyGlitch", &applyGlitch);
    function("applyThermal", &applyThermal);
    function("applyNightVision", &applyNightVision);
    function("applyToon", &applyToon);
    function("applyMirror", &applyMirror);
    function("applyOldTV", &applyOldTV);
    function("applyVHS", &applyVHS);
    function("applyChromaKey", &applyChromaKey);
    function("applyXrayFilter", &applyXrayFilter);
    function("allocateBuffer", &allocateBuffer);
    function("freeBuffer", &freeBuffer);
}