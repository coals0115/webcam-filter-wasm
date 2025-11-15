# 개발 워크플로우 가이드

> 실시간 웹캠 필터 구현을 위한 단계별 개발 가이드

이 문서는 WebAssembly로 웹캠 필터를 개발하는 전체 과정을 다룹니다.

---

## 📋 목차

1. [개발 프로세스 개요](#개발-프로세스-개요)
2. [프로젝트 초기 설정](#프로젝트-초기-설정)
3. [C++ 필터 구현](#c-필터-구현)
4. [JavaScript 바인딩](#javascript-바인딩)
5. [웹 인터페이스 구현](#웹-인터페이스-구현)
6. [빌드 및 테스트](#빌드-및-테스트)
7. [성능 최적화](#성능-최적화)
8. [디버깅 가이드](#디버깅-가이드)

---

## 개발 프로세스 개요

### WebAssembly 개발 흐름

```
[C++ 코드 작성]
    ↓
[emcc로 컴파일] → [.wasm + .js 생성]
    ↓
[JavaScript에서 WASM 로드]
    ↓
[웹 인터페이스와 연동]
    ↓
[브라우저에서 테스트]
```

### Spring Boot와 비교

| WebAssembly 개발 | Spring Boot 개발 |
|-----------------|-----------------|
| C++ 코드 작성 | Java 코드 작성 |
| `emcc` 컴파일 | `javac` or Maven/Gradle 빌드 |
| `.wasm` 바이너리 | `.jar` 파일 |
| JavaScript로 호출 | REST API 호출 |
| 브라우저에서 실행 | 서버에서 실행 |

---

## 프로젝트 초기 설정

### Step 1: 디렉토리 구조 생성

```bash
# 프로젝트 루트로 이동
cd ~/CLionProjects/webcam-filter-wasm

# 필요한 디렉토리 생성
mkdir -p src/filters
mkdir -p web
mkdir -p build
mkdir -p docs

# 기존 CLion 자동 생성 파일 정리
mv main.cpp src/
```

### Step 2: 빌드 스크립트 작성

프로젝트 루트에 `build.sh` 생성:

```bash
#!/bin/bash

# 빌드 스크립트 (build.sh)
# 용도: C++ 코드를 WebAssembly로 컴파일

# Emscripten 환경 변수 로드
source ./emsdk/emsdk_env.sh > /dev/null 2>&1

# 빌드 디렉토리 생성
mkdir -p build

# 컴파일 옵션 설명:
# -O3: 최적화 레벨 3 (성능 우선)
# -s WASM=1: WebAssembly 출력
# -s ALLOW_MEMORY_GROWTH=1: 메모리 동적 확장 허용
# -s EXPORTED_FUNCTIONS: JavaScript에서 호출 가능한 함수
# -s EXPORTED_RUNTIME_METHODS: 런타임 메소드 export
# --bind: Embind 사용 (C++ ↔ JavaScript 바인딩)

emcc src/main.cpp \
  src/filters/*.cpp \
  -O3 \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_RUNTIME_METHODS='["cwrap", "ccall"]' \
  --bind \
  -o build/filter.js

echo "✅ 빌드 완료: build/filter.js, build/filter.wasm"
```

**실행 권한 부여**:
```bash
chmod +x build.sh
```

### Step 3: 개발 서버 스크립트 작성

프로젝트 루트에 `serve.sh` 생성:

```bash
#!/bin/bash

# 개발 서버 스크립트 (serve.sh)
# 용도: 빌드 후 로컬 웹 서버 실행

echo "🌐 웹 서버 실행 중..."
echo "👉 브라우저에서 http://localhost:8080/index.html 접속"
echo "종료하려면 Ctrl+C"
cd build && python3 -m http.server 8080
```

**실행 권한 부여**:
```bash
chmod +x serve.sh
```

---

## C++ 필터 구현

### 기본 개념: 이미지 데이터 처리

웹캠 이미지는 **RGBA 배열**로 표현됩니다:

```
픽셀 구조: [R, G, B, A, R, G, B, A, ...]
           ↑ 1번 픽셀  ↑ 2번 픽셀

- R (Red): 0-255
- G (Green): 0-255
- B (Blue): 0-255
- A (Alpha): 0-255 (투명도, 보통 255)
```

**Java 비유**: `byte[]` 배열과 유사하지만, 웹에서는 `Uint8ClampedArray`로 표현됩니다.

### 필터 1: 흑백 (Grayscale)

**원리**: RGB 값을 가중 평균하여 밝기 계산

`src/filters/grayscale.cpp`:

```cpp
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

/**
 * 흑백 필터 적용
 * @param imageData JavaScript의 Uint8ClampedArray (RGBA 픽셀 배열)
 * @param width 이미지 너비
 * @param height 이미지 높이
 */
void applyGrayscale(val imageData, int width, int height) {
    // JavaScript 배열의 길이 확인
    int length = imageData["length"].as<int>();

    // 4픽셀씩 처리 (R, G, B, A)
    for (int i = 0; i < length; i += 4) {
        // 현재 픽셀의 RGB 값 추출
        int r = imageData[i].as<int>();
        int g = imageData[i + 1].as<int>();
        int b = imageData[i + 2].as<int>();

        // 밝기 계산 (ITU-R BT.709 표준)
        // 인간 눈의 색 민감도 고려: 녹색 > 빨강 > 파랑
        int gray = static_cast<int>(
            0.2126 * r +
            0.7152 * g +
            0.0722 * b
        );

        // RGB를 동일한 밝기 값으로 설정
        imageData.set(i, gray);       // R
        imageData.set(i + 1, gray);   // G
        imageData.set(i + 2, gray);   // B
        // Alpha (i + 3)는 그대로 유지
    }
}

// JavaScript에서 호출 가능하도록 바인딩
EMSCRIPTEN_BINDINGS(grayscale_module) {
    function("applyGrayscale", &applyGrayscale);
}
```

**Spring Boot 비유**:
```java
// Java에서는 이렇게 작성할 것을:
public void applyGrayscale(byte[] imageData, int width, int height) {
    for (int i = 0; i < imageData.length; i += 4) {
        int gray = (int)(0.2126 * imageData[i] + ...);
        imageData[i] = imageData[i+1] = imageData[i+2] = (byte)gray;
    }
}
```

### 필터 2: 블러 (Blur)

**원리**: 주변 픽셀의 평균값 계산 (Box Blur)

`src/filters/blur.cpp`:

```cpp
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <vector>

using namespace emscripten;

/**
 * 블러 필터 적용 (Box Blur 알고리즘)
 * @param imageData 원본 이미지 데이터
 * @param width 이미지 너비
 * @param height 이미지 높이
 * @param radius 블러 반경 (기본값 3)
 */
void applyBlur(val imageData, int width, int height, int radius = 3) {
    int length = imageData["length"].as<int>();

    // 원본 데이터 복사 (블러 계산을 위해 필요)
    std::vector<int> original(length);
    for (int i = 0; i < length; i++) {
        original[i] = imageData[i].as<int>();
    }

    // 각 픽셀에 대해 블러 적용
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int r = 0, g = 0, b = 0, count = 0;

            // 주변 픽셀 탐색 (radius 범위)
            for (int dy = -radius; dy <= radius; dy++) {
                for (int dx = -radius; dx <= radius; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;

                    // 이미지 경계 체크
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        int idx = (ny * width + nx) * 4;
                        r += original[idx];
                        g += original[idx + 1];
                        b += original[idx + 2];
                        count++;
                    }
                }
            }

            // 평균값 계산 및 적용
            int idx = (y * width + x) * 4;
            imageData.set(idx, r / count);
            imageData.set(idx + 1, g / count);
            imageData.set(idx + 2, b / count);
        }
    }
}

EMSCRIPTEN_BINDINGS(blur_module) {
    function("applyBlur", &applyBlur);
}
```

**복잡도 참고**:
- 시간 복잡도: O(width × height × radius²)
- 640×480 이미지, radius=3 → 약 1,840만 연산
- **최적화 필요**: 가우시안 블러, 분리 가능 필터 등 고급 기법 적용 가능

### 필터 3: 엣지 검출 (Edge Detection)

**원리**: Sobel 연산자를 사용한 경계선 검출

`src/filters/edge.cpp`:

```cpp
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <vector>
#include <cmath>

using namespace emscripten;

/**
 * 엣지 검출 필터 (Sobel Edge Detection)
 * @param imageData 원본 이미지 데이터
 * @param width 이미지 너비
 * @param height 이미지 높이
 */
void applyEdgeDetection(val imageData, int width, int height) {
    int length = imageData["length"].as<int>();

    // 1. 먼저 흑백 변환 (엣지 검출은 보통 흑백 이미지에서 수행)
    std::vector<int> gray(width * height);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;
            int r = imageData[idx].as<int>();
            int g = imageData[idx + 1].as<int>();
            int b = imageData[idx + 2].as<int>();
            gray[y * width + x] = static_cast<int>(
                0.2126 * r + 0.7152 * g + 0.0722 * b
            );
        }
    }

    // 2. Sobel 커널 정의
    int sobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int sobelY[3][3] = {
        {-1, -2, -1},
        { 0,  0,  0},
        { 1,  2,  1}
    };

    // 3. 엣지 검출 수행
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int gx = 0, gy = 0;

            // 3×3 커널 적용
            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    int pixel = gray[(y + ky) * width + (x + kx)];
                    gx += pixel * sobelX[ky + 1][kx + 1];
                    gy += pixel * sobelY[ky + 1][kx + 1];
                }
            }

            // 그래디언트 크기 계산
            int magnitude = static_cast<int>(std::sqrt(gx * gx + gy * gy));

            // 값 제한 (0-255)
            magnitude = std::min(255, std::max(0, magnitude));

            // 결과 적용
            int idx = (y * width + x) * 4;
            imageData.set(idx, magnitude);
            imageData.set(idx + 1, magnitude);
            imageData.set(idx + 2, magnitude);
        }
    }
}

EMSCRIPTEN_BINDINGS(edge_module) {
    function("applyEdgeDetection", &applyEdgeDetection);
}
```

**Sobel 연산자 설명**:
- **수평 엣지**: `sobelX` 커널로 검출 (좌우 밝기 변화)
- **수직 엣지**: `sobelY` 커널로 검출 (상하 밝기 변화)
- **최종 결과**: √(Gx² + Gy²) → 엣지 강도

### 메인 파일 통합

`src/main.cpp`:

```cpp
#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace emscripten;

// 필터 함수 선언 (각 .cpp에서 정의됨)
extern void applyGrayscale(val imageData, int width, int height);
extern void applyBlur(val imageData, int width, int height, int radius);
extern void applyEdgeDetection(val imageData, int width, int height);

/**
 * 버전 정보 반환
 */
std::string getVersion() {
    return "WebCam Filter WASM v1.0.0";
}

/**
 * 성능 측정용 타임스탬프 (밀리초)
 */
double getTimestamp() {
    return val::global("performance").call<double>("now");
}

// JavaScript 바인딩
EMSCRIPTEN_BINDINGS(main_module) {
    function("getVersion", &getVersion);
    function("getTimestamp", &getTimestamp);
}
```

---

## JavaScript 바인딩

### WASM 모듈 로드

`web/app.js`:

```javascript
// WASM 모듈 로드 상태
let wasmModule = null;
let isWasmReady = false;

/**
 * WebAssembly 모듈 초기화
 */
async function initWasm() {
    try {
        // Emscripten이 생성한 JS 파일 로드
        wasmModule = await Module();
        isWasmReady = true;

        console.log('✅ WASM 로드 완료:', wasmModule.getVersion());

        // 사용 가능한 필터 함수 확인
        console.log('사용 가능한 필터:', {
            grayscale: typeof wasmModule.applyGrayscale,
            blur: typeof wasmModule.applyBlur,
            edge: typeof wasmModule.applyEdgeDetection
        });

    } catch (error) {
        console.error('❌ WASM 로드 실패:', error);
        alert('WebAssembly 로드 실패. 콘솔을 확인하세요.');
    }
}

// 페이지 로드 시 WASM 초기화
initWasm();
```

**Spring Boot 비유**:
```java
// Spring Boot에서 Bean 초기화와 유사
@PostConstruct
public void init() {
    filterService = new FilterService();
    logger.info("Filter service initialized");
}
```

### 웹캠 스트림 처리

```javascript
// 웹캠 스트림 변수
let videoStream = null;
let videoElement = null;
let canvasElement = null;
let ctx = null;

// 선택된 필터
let currentFilter = 'none';

/**
 * 웹캠 초기화 및 스트림 시작
 */
async function startWebcam() {
    try {
        // 비디오 엘리먼트 참조
        videoElement = document.getElementById('webcam');
        canvasElement = document.getElementById('canvas');
        ctx = canvasElement.getContext('2d');

        // 웹캠 접근 권한 요청
        videoStream = await navigator.mediaDevices.getUserMedia({
            video: {
                width: { ideal: 640 },
                height: { ideal: 480 },
                facingMode: 'user'
            }
        });

        // 비디오 엘리먼트에 스트림 연결
        videoElement.srcObject = videoStream;
        videoElement.play();

        // 비디오 메타데이터 로드 후 캔버스 크기 설정
        videoElement.onloadedmetadata = () => {
            canvasElement.width = videoElement.videoWidth;
            canvasElement.height = videoElement.videoHeight;

            console.log('✅ 웹캠 시작:', {
                width: videoElement.videoWidth,
                height: videoElement.videoHeight
            });

            // 프레임 처리 시작
            processFrame();
        };

    } catch (error) {
        console.error('❌ 웹캠 접근 실패:', error);
        alert('웹캠 접근 권한이 필요합니다.');
    }
}

/**
 * 프레임 단위 처리 (실시간 렌더링)
 */
function processFrame() {
    if (!videoElement || !canvasElement) return;

    // 1. 비디오 프레임을 캔버스에 그리기
    ctx.drawImage(videoElement, 0, 0);

    // 2. 이미지 데이터 추출
    const imageData = ctx.getImageData(
        0, 0,
        canvasElement.width,
        canvasElement.height
    );

    // 3. WASM 필터 적용
    if (isWasmReady && currentFilter !== 'none') {
        applyFilter(imageData);
    }

    // 4. 처리된 이미지 다시 그리기
    ctx.putImageData(imageData, 0, 0);

    // 5. 다음 프레임 요청 (60 FPS 목표)
    requestAnimationFrame(processFrame);
}

/**
 * 필터 적용 (성능 측정 포함)
 */
function applyFilter(imageData) {
    const width = canvasElement.width;
    const height = canvasElement.height;

    // 성능 측정 시작
    const startTime = wasmModule.getTimestamp();

    // 필터 적용
    switch (currentFilter) {
        case 'grayscale':
            wasmModule.applyGrayscale(imageData.data, width, height);
            break;
        case 'blur':
            wasmModule.applyBlur(imageData.data, width, height, 3);
            break;
        case 'edge':
            wasmModule.applyEdgeDetection(imageData.data, width, height);
            break;
    }

    // 성능 측정 종료
    const endTime = wasmModule.getTimestamp();
    const processingTime = endTime - startTime;

    // 성능 표시 업데이트
    updatePerformanceStats(processingTime);
}

/**
 * 성능 통계 업데이트
 */
let frameCount = 0;
let totalTime = 0;

function updatePerformanceStats(processingTime) {
    frameCount++;
    totalTime += processingTime;

    // 30프레임마다 평균 계산
    if (frameCount % 30 === 0) {
        const avgTime = totalTime / frameCount;
        const fps = 1000 / avgTime;

        document.getElementById('stats').textContent =
            `처리 시간: ${avgTime.toFixed(2)}ms | FPS: ${fps.toFixed(1)}`;

        // 리셋
        frameCount = 0;
        totalTime = 0;
    }
}

/**
 * 필터 변경
 */
function changeFilter(filterName) {
    currentFilter = filterName;
    console.log('필터 변경:', filterName);
}
```

---

## 웹 인터페이스 구현

### HTML 구조

`web/index.html`:

```html
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WebCam Filter WASM</title>
    <link rel="stylesheet" href="styles.css">
</head>
<body>
    <div class="container">
        <h1>🎥 실시간 웹캠 필터</h1>

        <!-- 웹캠 영역 -->
        <div class="video-container">
            <!-- 원본 비디오 (숨김) -->
            <video id="webcam" autoplay playsinline style="display: none;"></video>

            <!-- 필터 적용된 캔버스 -->
            <canvas id="canvas"></canvas>
        </div>

        <!-- 필터 선택 -->
        <div class="controls">
            <button onclick="startWebcam()">📷 웹캠 시작</button>
            <button onclick="changeFilter('none')">원본</button>
            <button onclick="changeFilter('grayscale')">흑백</button>
            <button onclick="changeFilter('blur')">블러</button>
            <button onclick="changeFilter('edge')">엣지</button>
        </div>

        <!-- 성능 통계 -->
        <div class="stats">
            <p id="stats">대기 중...</p>
        </div>
    </div>

    <!-- WASM 모듈 로드 -->
    <script src="filter.js"></script>
    <!-- 앱 로직 -->
    <script src="app.js"></script>
</body>
</html>
```

### CSS 스타일

`web/styles.css`:

```css
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    display: flex;
    justify-content: center;
    align-items: center;
    padding: 20px;
}

.container {
    background: white;
    border-radius: 20px;
    padding: 30px;
    box-shadow: 0 20px 60px rgba(0, 0, 0, 0.3);
    max-width: 800px;
    width: 100%;
}

h1 {
    text-align: center;
    color: #333;
    margin-bottom: 20px;
}

.video-container {
    position: relative;
    background: #000;
    border-radius: 10px;
    overflow: hidden;
    margin-bottom: 20px;
}

#canvas {
    width: 100%;
    height: auto;
    display: block;
}

.controls {
    display: flex;
    gap: 10px;
    flex-wrap: wrap;
    justify-content: center;
    margin-bottom: 20px;
}

button {
    padding: 12px 24px;
    font-size: 16px;
    border: none;
    border-radius: 8px;
    background: #667eea;
    color: white;
    cursor: pointer;
    transition: all 0.3s ease;
    font-weight: 600;
}

button:hover {
    background: #764ba2;
    transform: translateY(-2px);
    box-shadow: 0 4px 12px rgba(0, 0, 0, 0.2);
}

button:active {
    transform: translateY(0);
}

.stats {
    background: #f5f5f5;
    padding: 15px;
    border-radius: 8px;
    text-align: center;
}

.stats p {
    color: #666;
    font-family: 'Courier New', monospace;
    font-size: 14px;
}

/* 반응형 디자인 */
@media (max-width: 600px) {
    .container {
        padding: 20px;
    }

    button {
        padding: 10px 20px;
        font-size: 14px;
    }
}
```

---

## 빌드 및 테스트

### 전체 빌드 프로세스

```bash
# 1. 프로젝트 루트로 이동
cd ~/CLionProjects/webcam-filter-wasm

# 2. Emscripten 환경 활성화 (필요시)
source ./emsdk/emsdk_env.sh

# 3. C++ → WASM 빌드
./build.sh

# 4. 웹 파일 복사
cp web/* build/

# 5. 개발 서버 실행
./serve.sh
```

**자동화 스크립트** (`dev.sh`):

```bash
#!/bin/bash

# 개발 워크플로우 자동화

echo "🔨 빌드 중..."
./build.sh

if [ $? -eq 0 ]; then
    echo "📦 웹 파일 복사 중..."
    cp web/* build/

    echo "✅ 빌드 완료!"
    echo "🌐 서버 시작..."
    ./serve.sh
else
    echo "❌ 빌드 실패!"
    exit 1
fi
```

### 테스트 체크리스트

**기능 테스트**:
- [ ] 웹캠 권한 요청 정상 작동
- [ ] 비디오 스트림 정상 출력
- [ ] 흑백 필터 정상 작동
- [ ] 블러 필터 정상 작동
- [ ] 엣지 검출 정상 작동
- [ ] 필터 전환 즉시 반영

**성능 테스트**:
- [ ] 640×480 해상도에서 30 FPS 이상 유지
- [ ] 각 필터의 처리 시간 <33ms (30 FPS 기준)
- [ ] CPU 사용률 <50%
- [ ] 메모리 누수 없음 (장시간 실행 테스트)

**브라우저 호환성**:
- [ ] Chrome/Edge (권장)
- [ ] Firefox
- [ ] Safari (macOS)

---

## 성능 최적화

### 1. 컴파일 최적화

**최적화 레벨 비교**:

```bash
# -O0: 최적화 없음 (디버그용)
emcc src/main.cpp -O0 -o build/filter.js

# -O2: 균형 잡힌 최적화 (개발 단계)
emcc src/main.cpp -O2 -o build/filter.js

# -O3: 최대 성능 (프로덕션)
emcc src/main.cpp -O3 -o build/filter.js

# -Oz: 최소 크기 (대역폭 제한 환경)
emcc src/main.cpp -Oz -o build/filter.js
```

### 2. SIMD 활용 (고급)

```bash
# SIMD (Single Instruction Multiple Data) 활성화
emcc src/main.cpp \
  -O3 \
  -msimd128 \
  -o build/filter.js
```

**SIMD란?**: 하나의 명령으로 여러 데이터를 동시 처리 (벡터 연산)

### 3. 멀티스레딩 (고급)

```bash
# Web Workers를 통한 병렬 처리
emcc src/main.cpp \
  -O3 \
  -s USE_PTHREADS=1 \
  -s PTHREAD_POOL_SIZE=4 \
  -o build/filter.js
```

**주의**: CORS 정책 및 SharedArrayBuffer 요구사항 확인 필요

---

## 디버깅 가이드

### 1. C++ 디버깅

**디버그 빌드**:
```bash
emcc src/main.cpp \
  -g \
  -O0 \
  -s ASSERTIONS=1 \
  -s SAFE_HEAP=1 \
  -o build/filter.js
```

**소스 맵 활성화**:
```bash
emcc src/main.cpp \
  -g \
  -gsource-map \
  --source-map-base http://localhost:8080/ \
  -o build/filter.js
```

브라우저 DevTools에서 C++ 소스 코드 직접 확인 가능!

### 2. JavaScript 디버깅

**콘솔 로그 추가**:
```javascript
// app.js에서
function applyFilter(imageData) {
    console.log('필터 적용:', currentFilter);
    console.log('이미지 크기:', imageData.width, 'x', imageData.height);

    const startTime = performance.now();
    // ... 필터 적용
    const endTime = performance.now();

    console.log('처리 시간:', (endTime - startTime).toFixed(2), 'ms');
}
```

### 3. 성능 프로파일링

**Chrome DevTools 사용**:
1. `Performance` 탭 열기
2. 녹화 시작 (Record)
3. 필터 적용
4. 녹화 중지
5. `Main` 스레드에서 `applyGrayscale` 등 함수 시간 확인

**메모리 누수 확인**:
1. `Memory` 탭 열기
2. `Heap snapshot` 촬영
3. 필터 여러 번 실행
4. 다시 `Heap snapshot` 촬영
5. `Comparison` 모드로 메모리 증가 확인

---

## 다음 단계

### 추가 기능 아이디어

1. **더 많은 필터**:
   - 세피아 (Sepia)
   - 반전 (Invert)
   - 선명도 (Sharpen)
   - 모자이크 (Pixelate)

2. **고급 기능**:
   - 얼굴 인식 (OpenCV WASM)
   - 배경 제거 (Segmentation)
   - 실시간 AR 효과

3. **UX 개선**:
   - 슬라이더로 필터 강도 조절
   - 사진 캡처 및 다운로드
   - 필터 프리셋 저장

### 학습 자료

- [Emscripten 공식 튜토리얼](https://emscripten.org/docs/getting_started/Tutorial.html)
- [WebAssembly 성능 최적화 가이드](https://web.dev/webassembly/)
- [이미지 처리 알고리즘](https://www.tutorialspoint.com/dip/index.htm)

---

**작성일**: 2025-11-08
**업데이트**: 필요시 지속적으로 개선 예정
