# 프로젝트 아키텍처

> 프로젝트 구조와 데이터 흐름을 초보자도 이해할 수 있도록 설명합니다.

## 목차
- [프로젝트 구조 개요](#프로젝트-구조-개요)
- [파일별 상세 설명](#파일별-상세-설명)
- [데이터 흐름](#데이터-흐름)
- [메모리 관리](#메모리-관리)
- [빌드 프로세스](#빌드-프로세스)
- [실행 흐름](#실행-흐름)

---

## 프로젝트 구조 개요

### 디렉토리 구조

```
webcam-filter-wasm/
│
├── 📁 src/                    # C++ 소스 코드 (현재 사용 안 함)
├── 📁 web/                    # 웹 프론트엔드
│   ├── index.html            # 메인 HTML 페이지
│   ├── styles.css            # 스타일시트
│   ├── app.js                # 메인 JavaScript 로직
│   └── wrapper.js            # WASM 로딩 래퍼
│
├── 📁 build/                  # 빌드 출력물
│   ├── filter.js             # 생성된 JavaScript 래퍼
│   └── filter.wasm           # 컴파일된 WebAssembly
│
├── 📁 docs/                   # 문서
│   ├── CONCEPTS.md           # 기초 개념
│   ├── GETTING_STARTED.md    # 시작 가이드
│   ├── ARCHITECTURE.md       # 이 문서
│   └── ...
│
├── main.cpp                  # C++ 메인 소스 파일
├── CMakeLists.txt            # CMake 빌드 설정
├── build.sh                  # 빌드 스크립트
├── serve.sh                  # 개발 서버 스크립트
└── README.md                 # 프로젝트 설명
```

### 계층 구조

```
┌─────────────────────────────────────────┐
│         사용자 인터페이스 (HTML)          │ ← 웹캠 영상 표시
└──────────────────┬──────────────────────┘
                   │
┌──────────────────┴──────────────────────┐
│    JavaScript 계층 (app.js)             │ ← 웹캠 제어, UI 이벤트
└──────────────────┬──────────────────────┘
                   │
┌──────────────────┴──────────────────────┐
│   WASM 래퍼 (filter.js, wrapper.js)     │ ← JS ↔ WASM 브릿지
└──────────────────┬──────────────────────┘
                   │
┌──────────────────┴──────────────────────┐
│  WebAssembly (filter.wasm)              │ ← 고성능 이미지 처리
└──────────────────┬──────────────────────┘
                   │
┌──────────────────┴──────────────────────┐
│    C++ 소스 (main.cpp)                   │ ← 필터 알고리즘
└─────────────────────────────────────────┘
```

---

## 파일별 상세 설명

### 1. main.cpp (C++ 소스)

**역할**: 이미지 처리 알고리즘 구현

**위치**: `/main.cpp`

**주요 내용**:
```cpp
// 1. Emscripten 헤더 포함
#include <emscripten/bind.h>
#include <emscripten/val.h>

// 2. 필터 함수 구현
void applyGrayscaleFilterRaw(uintptr_t ptr, int length) {
    // 흑백 필터 로직
}

void applyHorizontalFlipRaw(uintptr_t ptr, int width, int height) {
    // 좌우반전 로직
}

// 3. JavaScript 바인딩
EMSCRIPTEN_BINDINGS(webcam_filter) {
    function("applyGrayscaleFilterRaw", &applyGrayscaleFilterRaw);
    function("applyHorizontalFlipRaw", &applyHorizontalFlipRaw);
}
```

**핵심 기능**:
- `applyGrayscaleFilterRaw()`: 이미지를 흑백으로 변환
- `applyHorizontalFlipRaw()`: 이미지를 좌우반전
- `EMSCRIPTEN_BINDINGS`: JavaScript에서 호출 가능하도록 함수 노출

**왜 이 파일이 중요한가?**:
- 실제 필터 알고리즘이 구현된 곳
- 성능이 중요한 연산을 C++로 처리
- 새로운 필터를 추가하려면 이 파일을 수정

---

### 2. CMakeLists.txt (빌드 설정)

**역할**: CMake 빌드 시스템 설정

**위치**: `/CMakeLists.txt`

**주요 내용**:
```cmake
cmake_minimum_required(VERSION 3.10)
project(webcam_filter)

# C++17 표준 사용
set(CMAKE_CXX_STANDARD 17)

# Emscripten 바인딩 활성화
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --bind")

# 실행 파일 생성
add_executable(filter main.cpp)
```

**핵심 역할**:
- C++ 컴파일 옵션 설정
- Emscripten 컴파일러 설정
- 출력 파일 이름 지정

---

### 3. build.sh (빌드 스크립트)

**역할**: 컴파일 자동화

**위치**: `/build.sh`

**주요 내용**:
```bash
#!/bin/bash

# build 디렉토리 생성
mkdir -p build
cd build

# CMake 설정 (Emscripten 사용)
emcmake cmake ..

# 빌드 실행
emmake make

# 생성된 파일 복사
cp filter.js filter.wasm ../web/
```

**실행 방법**:
```bash
chmod +x build.sh
./build.sh
```

**생성되는 파일**:
- `build/filter.js`: JavaScript 래퍼
- `build/filter.wasm`: WebAssembly 바이너리
- → `web/` 디렉토리로 자동 복사

---

### 4. web/index.html (메인 페이지)

**역할**: 사용자 인터페이스

**위치**: `/web/index.html`

**주요 구조**:
```html
<!DOCTYPE html>
<html>
<head>
    <title>Webcam Filter (WASM)</title>
    <link rel="stylesheet" href="styles.css">
</head>
<body>
    <!-- 웹캠 영상 표시 -->
    <video id="webcam" autoplay playsinline></video>

    <!-- 캔버스 (필터 적용 결과) -->
    <canvas id="canvas"></canvas>

    <!-- 필터 버튼 -->
    <button id="grayscaleBtn">흑백 필터</button>
    <button id="mirrorBtn">좌우반전</button>

    <!-- JavaScript 로딩 -->
    <script src="wrapper.js"></script>
    <script src="app.js"></script>
</body>
</html>
```

**핵심 요소**:
- `<video>`: 웹캠 스트림 표시
- `<canvas>`: 필터 적용 후 결과 표시
- `<button>`: 필터 선택 UI

---

### 5. web/app.js (메인 로직)

**역할**: 웹캠 제어 및 필터 적용 로직

**위치**: `/web/app.js`

**주요 함수**:

#### (1) 웹캠 초기화
```javascript
async function initWebcam() {
    const stream = await navigator.mediaDevices.getUserMedia({
        video: { width: 1280, height: 720 }
    });
    video.srcObject = stream;
}
```

#### (2) 프레임 처리 루프
```javascript
function processFrame() {
    // 1. 웹캠에서 프레임 가져오기
    ctx.drawImage(video, 0, 0, width, height);

    // 2. ImageData 추출
    const imageData = ctx.getImageData(0, 0, width, height);

    // 3. WASM으로 필터 적용
    if (currentFilter === 'grayscale') {
        applyGrayscaleFilter(imageData);
    } else if (currentFilter === 'mirror') {
        applyMirrorFilter(imageData);
    }

    // 4. Canvas에 다시 그리기
    ctx.putImageData(imageData, 0, 0);

    // 5. 다음 프레임 요청
    requestAnimationFrame(processFrame);
}
```

#### (3) WASM 함수 호출
```javascript
function applyGrayscaleFilter(imageData) {
    const bytes = imageData.data;
    const numBytes = bytes.length;

    // WASM 메모리 할당
    const ptr = Module._malloc(numBytes);

    // 데이터 복사
    Module.HEAPU8.set(bytes, ptr);

    // C++ 함수 호출
    Module._applyGrayscaleFilterRaw(ptr, numBytes);

    // 결과 가져오기
    bytes.set(Module.HEAPU8.subarray(ptr, ptr + numBytes));

    // 메모리 해제
    Module._free(ptr);
}
```

---

### 6. web/wrapper.js (WASM 로딩)

**역할**: WebAssembly 모듈 로딩 및 초기화

**위치**: `/web/wrapper.js`

**주요 내용**:
```javascript
var Module = {
    onRuntimeInitialized: function() {
        console.log('WebAssembly module loaded');
        // app.js의 초기화 함수 호출
        if (typeof onWasmReady === 'function') {
            onWasmReady();
        }
    }
};
```

**실행 순서**:
```
1. index.html 로드
2. wrapper.js 실행 → Module 객체 설정
3. filter.js 로드 → WASM 초기화
4. filter.wasm 로드 → 바이너리 로딩
5. onRuntimeInitialized 콜백 실행
6. app.js에서 웹캠 시작
```

---

### 7. web/styles.css (스타일시트)

**역할**: UI 디자인

**위치**: `/web/styles.css`

**주요 스타일**:
```css
/* 전체 레이아웃 */
body {
    display: flex;
    flex-direction: column;
    align-items: center;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
}

/* 캔버스 스타일 */
canvas {
    max-width: 90vw;
    border-radius: 12px;
    box-shadow: 0 10px 40px rgba(0,0,0,0.3);
}

/* 버튼 스타일 */
button {
    padding: 12px 32px;
    border-radius: 8px;
    background: white;
    transition: all 0.3s ease;
}

button:hover {
    transform: translateY(-2px);
    box-shadow: 0 5px 20px rgba(0,0,0,0.2);
}
```

---

## 데이터 흐름

### 전체 데이터 파이프라인

```
┌─────────────────────────────────────────────────────────┐
│ 1. 웹캠 → Video Element                                 │
│    navigator.mediaDevices.getUserMedia()                │
│    - MediaStream 객체 생성                              │
│    - video.srcObject에 할당                             │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 2. Video → Canvas                                       │
│    ctx.drawImage(video, 0, 0, width, height)           │
│    - 현재 프레임을 Canvas에 그리기                      │
│    - 60fps로 반복 실행                                  │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 3. Canvas → ImageData                                   │
│    const imageData = ctx.getImageData(0, 0, w, h)      │
│    - 픽셀 데이터 추출                                   │
│    - Uint8ClampedArray 형식                            │
│    - [R, G, B, A, R, G, B, A, ...]                     │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 4. JavaScript → WASM Memory                             │
│    const ptr = Module._malloc(numBytes)                │
│    Module.HEAPU8.set(imageData.data, ptr)              │
│    - JavaScript 배열 → WASM 힙 메모리로 복사           │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 5. C++ 필터 처리                                        │
│    Module._applyGrayscaleFilterRaw(ptr, length)        │
│    - WASM 메모리 직접 수정                             │
│    - 각 픽셀에 필터 알고리즘 적용                       │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 6. WASM Memory → JavaScript                             │
│    imageData.data.set(                                  │
│        Module.HEAPU8.subarray(ptr, ptr + numBytes)     │
│    )                                                    │
│    - 처리된 데이터 JavaScript로 복사                    │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 7. ImageData → Canvas                                   │
│    ctx.putImageData(imageData, 0, 0)                   │
│    - 필터 적용된 이미지 화면에 표시                     │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 8. 반복                                                 │
│    requestAnimationFrame(processFrame)                 │
│    - 60fps로 반복 실행                                  │
└─────────────────────────────────────────────────────────┘
```

### 상세 메모리 맵

```
JavaScript 메모리:
┌─────────────────────────────────┐
│ ImageData.data                  │
│ [255, 128, 64, 255, ...]       │ ← Uint8ClampedArray
└─────────────────────────────────┘
         │
         ↓ (복사)
WebAssembly 힙:
┌─────────────────────────────────┐
│ Module.HEAPU8[ptr]              │
│ [255, 128, 64, 255, ...]       │ ← C++에서 직접 수정
└─────────────────────────────────┘
         │
         ↓ (수정)
┌─────────────────────────────────┐
│ Module.HEAPU8[ptr]              │
│ [128, 128, 128, 255, ...]      │ ← 필터 적용 결과
└─────────────────────────────────┘
         │
         ↓ (복사)
JavaScript 메모리:
┌─────────────────────────────────┐
│ ImageData.data                  │
│ [128, 128, 128, 255, ...]      │ ← 화면에 표시
└─────────────────────────────────┘
```

---

## 메모리 관리

### JavaScript ↔ WebAssembly 메모리 공유

#### 메모리 할당
```javascript
// 1. WASM 힙에 메모리 할당
const ptr = Module._malloc(numBytes);
// → C의 malloc()과 동일
// → 반환값: 메모리 주소 (포인터)
```

#### 데이터 복사 (JS → WASM)
```javascript
// 2. JavaScript 배열 → WASM 메모리로 복사
Module.HEAPU8.set(imageData.data, ptr);
// → HEAPU8: WASM 힙의 Uint8Array 뷰
// → set(): 배열 복사 메서드
```

#### 데이터 읽기 (WASM → JS)
```javascript
// 3. WASM 메모리 → JavaScript 배열로 복사
imageData.data.set(
    Module.HEAPU8.subarray(ptr, ptr + numBytes)
);
// → subarray(): 메모리 영역 참조
```

#### 메모리 해제
```javascript
// 4. WASM 메모리 해제 (필수!)
Module._free(ptr);
// → C의 free()와 동일
// → 메모리 누수 방지
```

### 메모리 누수 방지 패턴

```javascript
function safeApplyFilter(imageData) {
    let ptr = 0;  // 초기화

    try {
        const numBytes = imageData.data.length;
        ptr = Module._malloc(numBytes);  // 할당

        // 데이터 처리
        Module.HEAPU8.set(imageData.data, ptr);
        Module._applyGrayscaleFilterRaw(ptr, numBytes);
        imageData.data.set(Module.HEAPU8.subarray(ptr, ptr + numBytes));

    } finally {
        // 반드시 해제 (예외 발생 시에도)
        if (ptr !== 0) {
            Module._free(ptr);
        }
    }
}
```

---

## 빌드 프로세스

### 빌드 파이프라인

```
┌────────────────────────────────────────────┐
│ 1. main.cpp (C++ 소스 코드)                 │
└────────────────┬───────────────────────────┘
                 │
                 ↓ [emcc 컴파일러]
┌────────────────────────────────────────────┐
│ 2. LLVM IR (중간 표현)                      │
│    - C++을 LLVM 형식으로 변환               │
└────────────────┬───────────────────────────┘
                 │
                 ↓ [최적화]
┌────────────────────────────────────────────┐
│ 3. 최적화된 LLVM IR                         │
│    - -O3: 최고 수준 최적화                  │
│    - 인라인 확장, 루프 최적화               │
└────────────────┬───────────────────────────┘
                 │
                 ↓ [WebAssembly 생성]
┌────────────────┴───────────────────────────┐
│                                            │
│  ┌────────────────┐    ┌────────────────┐ │
│  │ filter.wasm    │    │ filter.js      │ │
│  │ (바이너리)     │    │ (래퍼 코드)    │ │
│  └────────────────┘    └────────────────┘ │
└────────────────────────────────────────────┘
                 │
                 ↓ [배포]
┌────────────────────────────────────────────┐
│ 4. web/ 디렉토리로 복사                     │
│    - 웹 서버에서 제공 가능                  │
└────────────────────────────────────────────┘
```

### 컴파일 옵션 상세

```bash
em++ main.cpp -o filter.js \
    --bind \                    # JavaScript 바인딩 활성화
    -O3 \                       # 최고 수준 최적화
    -s WASM=1 \                 # WebAssembly 생성
    -s ALLOW_MEMORY_GROWTH=1 \  # 메모리 동적 증가 허용
    -s MODULARIZE=1 \           # 모듈 형식으로 내보내기
    -s EXPORT_NAME="Module"     # 모듈 이름 지정
```

**옵션 설명**:
- `--bind`: Emscripten 바인딩 시스템 활성화
- `-O3`: 최고 수준 최적화 (속도 우선)
- `-s WASM=1`: WebAssembly 타겟으로 컴파일
- `-s ALLOW_MEMORY_GROWTH=1`: 메모리 자동 확장 허용
- `-s MODULARIZE=1`: ES6 모듈 형식 사용

---

## 실행 흐름

### 애플리케이션 시작 시퀀스

```
1. 페이지 로드
   ├─ index.html 파싱
   ├─ styles.css 로드 (UI 스타일)
   ├─ wrapper.js 실행 (Module 객체 설정)
   ├─ filter.js 로드 (WASM 래퍼)
   └─ filter.wasm 다운로드 (바이너리)

2. WASM 초기화
   ├─ filter.wasm 컴파일
   ├─ Module 객체 초기화
   ├─ 메모리 할당
   └─ onRuntimeInitialized 콜백 실행

3. 애플리케이션 시작
   ├─ app.js 실행
   ├─ 웹캠 권한 요청
   ├─ MediaStream 획득
   └─ 프레임 처리 루프 시작

4. 실시간 처리
   └─ (60fps 반복)
       ├─ 웹캠 → Canvas
       ├─ Canvas → ImageData
       ├─ JS → WASM 메모리 복사
       ├─ C++ 필터 적용
       ├─ WASM → JS 메모리 복사
       ├─ ImageData → Canvas
       └─ 화면에 표시
```

### 프레임 처리 타임라인 (16.67ms @ 60fps)

```
┌─────────────────────────────────────────────┐
│ 0ms: requestAnimationFrame 콜백 시작        │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 1ms: ctx.drawImage(video)                   │
│      - 웹캠 프레임 → Canvas (0.5ms)         │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 2ms: ctx.getImageData()                     │
│      - 픽셀 데이터 추출 (0.8ms)             │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 3ms: 메모리 할당 및 복사                    │
│      - _malloc() (0.1ms)                    │
│      - HEAPU8.set() (0.5ms)                 │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 4ms: C++ 필터 실행 (WASM)                   │
│      - applyGrayscaleFilterRaw() (2~3ms)    │
│      ⚡ 가장 많은 시간 소요                 │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 7ms: 결과 복사 및 메모리 해제               │
│      - subarray() + set() (0.5ms)           │
│      - _free() (0.1ms)                      │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 8ms: ctx.putImageData()                     │
│      - 처리된 이미지 → Canvas (0.8ms)       │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 9ms: requestAnimationFrame()                │
│      - 다음 프레임 예약                     │
└─────────────────────────────────────────────┘

총 소요 시간: ~9ms (16.67ms 예산의 54%)
여유 시간: ~7ms → 추가 필터나 효과 가능
```

---

## 성능 최적화 포인트

### 1. 메모리 재사용

❌ **비효율적** (매 프레임 할당/해제):
```javascript
function processFrame() {
    const ptr = Module._malloc(numBytes);  // 매번 할당
    // ... 처리
    Module._free(ptr);  // 매번 해제
    requestAnimationFrame(processFrame);
}
```

✅ **효율적** (메모리 재사용):
```javascript
let cachedPtr = 0;

function initMemory() {
    cachedPtr = Module._malloc(numBytes);  // 최초 1회만
}

function processFrame() {
    // cachedPtr 재사용
    Module.HEAPU8.set(imageData.data, cachedPtr);
    Module._applyGrayscaleFilterRaw(cachedPtr, numBytes);
    // ... 처리

    requestAnimationFrame(processFrame);
}

function cleanup() {
    Module._free(cachedPtr);  // 종료 시 1회 해제
}
```

### 2. 불필요한 복사 최소화

❌ **비효율적**:
```javascript
// 중간 배열 생성 (불필요한 메모리 사용)
const tempArray = new Uint8ClampedArray(imageData.data);
Module.HEAPU8.set(tempArray, ptr);
```

✅ **효율적**:
```javascript
// 직접 복사
Module.HEAPU8.set(imageData.data, ptr);
```

### 3. Canvas 작업 최적화

✅ **권장 사항**:
```javascript
// Canvas 크기를 고정하고 재사용
canvas.width = 1280;
canvas.height = 720;

// willReadFrequently 옵션 사용 (getImageData 최적화)
const ctx = canvas.getContext('2d', { willReadFrequently: true });
```

---

## 디버깅 팁

### 1. 성능 측정

```javascript
function processFrame() {
    const start = performance.now();

    // ... 필터 처리

    const end = performance.now();
    console.log(`Frame time: ${(end - start).toFixed(2)}ms`);
}
```

### 2. 메모리 사용량 확인

```javascript
// WASM 메모리 사용량 확인
console.log('WASM Memory:', Module.HEAPU8.length / (1024 * 1024), 'MB');
```

### 3. FPS 모니터링

```javascript
let lastTime = performance.now();
let frames = 0;

function processFrame() {
    frames++;
    const now = performance.now();

    if (now - lastTime >= 1000) {
        console.log('FPS:', frames);
        frames = 0;
        lastTime = now;
    }

    // ... 처리
    requestAnimationFrame(processFrame);
}
```

---

## 확장 포인트

### 새로운 필터 추가 시 수정 파일

1. **main.cpp**: C++ 필터 함수 구현
2. **app.js**: JavaScript에서 새 필터 호출 로직 추가
3. **index.html**: 필터 버튼 UI 추가

---

## 다음 단계

아키텍처를 이해했다면:

1. 🎓 [튜토리얼](TUTORIAL.md) - 첫 번째 필터 만들어보기
2. 📚 [API 레퍼런스](API_REFERENCE.md) - 함수 상세 설명
3. 🔧 [문제 해결](TROUBLESHOOTING.md) - 자주 발생하는 문제 해결

---

## 핵심 요약

### 3줄 요약

1. **C++ (main.cpp)**: 고성능 필터 알고리즘 구현
2. **WASM (filter.wasm)**: 브라우저에서 실행 가능한 바이너리
3. **JavaScript (app.js)**: 웹캠 제어 및 WASM 함수 호출

### 데이터 흐름

```
웹캠 → Canvas → ImageData → WASM 메모리
       ↓                            ↓
     화면 ← Canvas ← ImageData ← C++ 필터
```

### 파일 역할

| 파일 | 역할 | 언어 |
|------|------|------|
| main.cpp | 필터 알고리즘 | C++ |
| app.js | 웹캠 제어, 필터 호출 | JavaScript |
| wrapper.js | WASM 로딩 | JavaScript |
| filter.wasm | 컴파일된 바이너리 | WebAssembly |
| filter.js | JS ↔ WASM 브릿지 | JavaScript |
| index.html | UI | HTML |
| styles.css | 디자인 | CSS |
