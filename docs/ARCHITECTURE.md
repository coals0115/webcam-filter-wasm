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
├── 📁 src/                    # C++ 소스 코드
│   └── filters/              # 필터 구현
│       └── grayscale.cpp     # 흑백 + 좌우반전 필터
│
├── 📁 web/                    # 웹 프론트엔드
│   ├── index.html            # 메인 HTML 페이지
│   ├── styles.css            # 스타일시트
│   ├── app.js                # 메인 JavaScript 로직
│   └── wrapper.js            # WASM 로딩 래퍼
│
├── 📁 build/                  # 빌드 출력물
│   ├── filters.js            # 생성된 JavaScript 글루 코드
│   ├── filters.wasm          # 컴파일된 WebAssembly
│   └── (web 파일들 복사됨)    # index.html, app.js 등
│
├── 📁 docs/                   # 문서
│   ├── CONCEPTS.md           # 기초 개념
│   ├── GETTING_STARTED.md    # 시작 가이드
│   ├── ARCHITECTURE.md       # 이 문서
│   └── ...
│
├── main.cpp                  # 테스트용 Hello World
├── build.sh                  # 빌드 스크립트 (emcc 사용)
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

### 2. build.sh (빌드 스크립트)

**역할**: emcc를 사용한 컴파일 자동화

**위치**: `/build.sh`

**주요 내용**:
```bash
#!/bin/bash

# Emscripten 환경 활성화
cd emsdk && source ./emsdk_env.sh && cd ..

# build 디렉토리 생성
mkdir -p build

# C++ → WebAssembly 컴파일
emcc src/filters/grayscale.cpp -o build/filters.js \
  -O3 \
  --bind \
  -s WASM=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s MODULARIZE=1 \
  -s EXPORT_NAME="Module" \
  -s EXPORTED_RUNTIME_METHODS='["cwrap","ccall"]'

# 웹 파일 복사
cp web/* build/
```

**실행 방법**:
```bash
chmod +x build.sh
./build.sh
```

**주요 컴파일 옵션**:
- `-O3`: 최적화 레벨 3 (최고 성능)
- `--bind`: Emscripten embind 활성화 (C++ ↔ JS 바인딩)
- `-s WASM=1`: WebAssembly 출력 활성화
- `-s ALLOW_MEMORY_GROWTH=1`: 동적 메모리 증가 허용
- `-s MODULARIZE=1`: ES6 모듈로 내보내기
- `-s EXPORT_NAME="Module"`: 모듈 이름 지정

**생성되는 파일**:
- `build/filters.js`: JavaScript 글루 코드 (Emscripten 생성)
- `build/filters.wasm`: WebAssembly 바이너리 (컴파일된 C++ 코드)
- `build/index.html`, `build/app.js` 등: 웹 파일들 (복사본)

---

### 3. web/index.html (메인 페이지)

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
        video: {
            width: { ideal: 640 },
            height: { ideal: 480 },
            facingMode: 'user'
        },
        audio: false
    });
    video.srcObject = stream;

    // 비디오 메타데이터 로딩 완료 대기
    await new Promise((resolve) => {
        video.onloadedmetadata = resolve;
    });

    // 캔버스 크기 설정
    canvas.width = video.videoWidth;
    canvas.height = video.videoHeight;

    // WASM 버퍼 미리 할당 (영상 크기에 맞게)
    const bufferSize = canvas.width * canvas.height * 4;
    wasmBuffer = wasmModule.allocateBuffer(bufferSize);
}
```

#### (2) 프레임 처리 루프 (고성능 버전)
```javascript
function processFrame() {
    // 1. 웹캠에서 프레임 가져오기
    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);

    // 2. 필터 적용 (none이 아닐 때만)
    if (currentFilter !== 'none' && wasmModule && wasmBuffer) {
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        const data = imageData.data;

        // 3. JS 데이터를 WASM 메모리로 복사 (미리 할당된 버퍼 재사용)
        wasmModule.HEAPU8.set(data, wasmBuffer);

        // 4. WASM에서 필터 처리
        if (currentFilter === 'sepia') {
            wasmModule.applySepia(wasmBuffer, data.length);
        } else if (currentFilter === 'xray') {
            wasmModule.applyXrayFilter(wasmBuffer, data.length);
        } else if (currentFilter === 'mirror') {
            wasmModule.applyMirror(wasmBuffer, canvas.width, canvas.height, mode);
        } else if (currentFilter === 'pixelate') {
            wasmModule.applyPixelate(wasmBuffer, canvas.width, canvas.height, blockSize);
        } else if (currentFilter === 'chroma') {
            wasmModule.applyChromaKey(wasmBuffer, chromaBgBuffer, ...);
        } else if (currentFilter === 'thermal') {
            wasmModule.applyThermal(wasmBuffer, data.length);
        }

        // 5. WASM 메모리에서 JS로 결과 복사
        data.set(wasmModule.HEAPU8.subarray(wasmBuffer, wasmBuffer + data.length));
        ctx.putImageData(imageData, 0, 0);
    }

    // 6. 다음 프레임 요청
    requestAnimationFrame(processFrame);
}
```

**지원 필터 목록**:
- `none`: 원본 (필터 없음)
- `sepia`: 세피아 톤
- `xray`: X-Ray 효과
- `mirror`: 좌우/상하/4분할 반전
- `pixelate`: 픽셀화
- `chroma`: 크로마키 (배경 합성)
- `thermal`: 열화상 효과

#### (3) WASM 메모리 관리 (버퍼 재사용 방식)
```javascript
// 초기화 시 버퍼 할당 (1회)
const bufferSize = canvas.width * canvas.height * 4;
wasmBuffer = wasmModule.allocateBuffer(bufferSize);

// 프레임 처리 시 버퍼 재사용
wasmModule.HEAPU8.set(data, wasmBuffer);
wasmModule.applySepia(wasmBuffer, data.length);
data.set(wasmModule.HEAPU8.subarray(wasmBuffer, wasmBuffer + data.length));

// 종료 시 버퍼 해제
wasmModule.freeBuffer(wasmBuffer);
```

> **성능 개선**: 매 프레임 `_malloc()`/`_free()` 호출 대신 버퍼를 미리 할당하여 재사용합니다.

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
│    navigator.mediaDevices.getUserMedia({                │
│        video: { width: 640, height: 480, facingMode }   │
│    })                                                   │
│    - MediaStream 객체 생성                              │
│    - video.srcObject에 할당                             │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 2. Video → Canvas                                       │
│    ctx.drawImage(video, 0, 0, canvas.width, height)    │
│    - 현재 프레임을 Canvas에 그리기                      │
│    - requestAnimationFrame으로 반복 실행               │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 3. Canvas → ImageData (필터 적용 시에만)                │
│    const imageData = ctx.getImageData(0, 0, w, h)      │
│    - 픽셀 데이터 추출                                   │
│    - Uint8ClampedArray 형식                            │
│    - [R, G, B, A, R, G, B, A, ...]                     │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 4. JavaScript → WASM Memory (버퍼 재사용)               │
│    wasmModule.HEAPU8.set(data, wasmBuffer)             │
│    - 초기화 시 allocateBuffer()로 미리 할당            │
│    - 매 프레임 동일 버퍼 재사용 (성능 최적화)           │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 5. C++ 필터 처리 (다양한 필터 지원)                     │
│    wasmModule.applySepia(wasmBuffer, length)           │
│    wasmModule.applyXrayFilter(wasmBuffer, length)      │
│    wasmModule.applyMirror(wasmBuffer, w, h, mode)      │
│    wasmModule.applyPixelate(wasmBuffer, w, h, block)   │
│    wasmModule.applyChromaKey(wasmBuffer, bg, ...)      │
│    wasmModule.applyThermal(wasmBuffer, length)         │
└────────────────┬────────────────────────────────────────┘
                 │
                 ↓
┌─────────────────────────────────────────────────────────┐
│ 6. WASM Memory → JavaScript                             │
│    data.set(wasmModule.HEAPU8.subarray(                │
│        wasmBuffer, wasmBuffer + data.length            │
│    ))                                                   │
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
│    - FPS 및 처리 시간 측정 포함                         │
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
         ↓ (복사: HEAPU8.set())
WebAssembly 힙 (미리 할당된 버퍼):
┌─────────────────────────────────┐
│ wasmModule.HEAPU8[wasmBuffer]   │
│ [255, 128, 64, 255, ...]       │ ← C++에서 직접 수정
└─────────────────────────────────┘
         │
         ↓ (필터 적용: applySepia 등)
┌─────────────────────────────────┐
│ wasmModule.HEAPU8[wasmBuffer]   │
│ [180, 150, 120, 255, ...]      │ ← 세피아 필터 적용 결과
└─────────────────────────────────┘
         │
         ↓ (복사: subarray + set)
JavaScript 메모리:
┌─────────────────────────────────┐
│ ImageData.data                  │
│ [180, 150, 120, 255, ...]      │ ← 화면에 표시
└─────────────────────────────────┘

버퍼 관리 (성능 최적화):
┌─────────────────────────────────┐
│ wasmBuffer (초기화 시 1회 할당)  │
│ - allocateBuffer(bufferSize)    │
│ - 640 × 480 × 4 = 1,228,800    │
│ - 매 프레임 재사용              │
│ - 종료 시 freeBuffer() 해제     │
└─────────────────────────────────┘
```

---

## 메모리 관리

### JavaScript ↔ WebAssembly 메모리 공유

#### 현재 구현 방식 (버퍼 재사용)

```javascript
// 1. 초기화 시 버퍼 미리 할당 (1회)
const bufferSize = canvas.width * canvas.height * 4;
wasmBuffer = wasmModule.allocateBuffer(bufferSize);

// 2. 프레임 처리 시 버퍼 재사용
wasmModule.HEAPU8.set(data, wasmBuffer);           // JS → WASM
wasmModule.applySepia(wasmBuffer, data.length);   // 필터 적용
data.set(wasmModule.HEAPU8.subarray(              // WASM → JS
    wasmBuffer, wasmBuffer + data.length
));

// 3. 종료 시 버퍼 해제 (1회)
wasmModule.freeBuffer(wasmBuffer);
```

#### 기본 메모리 관리 API (참고)

```javascript
// WASM 힙에 메모리 할당 (저수준 API)
const ptr = Module._malloc(numBytes);
// → C의 malloc()과 동일
// → 반환값: 메모리 주소 (포인터)

// WASM 메모리 해제
Module._free(ptr);
// → C의 free()와 동일
// → 메모리 누수 방지
```

### 메모리 누수 방지 패턴 (현재 구현)

```javascript
// 페이지 언로드 시 정리
window.addEventListener('beforeunload', () => {
    // 애니메이션 중지
    if (animationId) cancelAnimationFrame(animationId);

    // WASM 버퍼 해제
    if (wasmModule && wasmBuffer) wasmModule.freeBuffer(wasmBuffer);
    if (wasmModule && chromaBgBuffer) wasmModule.freeBuffer(chromaBgBuffer);

    // 웹캠 스트림 정리
    if (video.srcObject) {
        video.srcObject.getTracks().forEach(track => track.stop());
    }
});
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
   ├─ filters.js 로드 (WASM 래퍼, MODULARIZE 모드)
   └─ app.js 로드 (메인 로직)

2. 앱 초기화 (init 함수)
   ├─ UI 애니메이션 시작 (페이드 인)
   ├─ 이벤트 리스너 등록 (필터 버튼, 크로마키 설정)
   └─ loadWasmModule() 호출

3. WASM 초기화 (loadWasmModule)
   ├─ Module() 함수 호출 (MODULARIZE 모드)
   ├─ filters.wasm 컴파일 및 초기화
   └─ wasmModule 인스턴스 저장

4. 웹캠 초기화 (initWebcam)
   ├─ getUserMedia() 호출 (640×480, user 카메라)
   ├─ video.srcObject에 스트림 할당
   ├─ 메타데이터 로딩 대기
   ├─ 캔버스 크기 설정
   ├─ WASM 버퍼 미리 할당 (allocateBuffer)
   └─ processFrame() 호출 → 루프 시작

5. 실시간 처리 (processFrame 루프)
   └─ requestAnimationFrame 반복
       ├─ 웹캠 → Canvas (drawImage)
       ├─ 필터 적용 (none이 아닐 경우)
       │   ├─ getImageData → WASM 복사
       │   ├─ 필터 함수 호출 (sepia, xray, mirror 등)
       │   └─ WASM → putImageData
       ├─ 처리 시간 측정 및 표시
       └─ FPS 계산 및 표시
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
│ 2ms: ctx.getImageData() (필터 시에만)       │
│      - 픽셀 데이터 추출 (0.8ms)             │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 3ms: WASM 메모리 복사                       │
│      - HEAPU8.set() (0.5ms)                 │
│      - 버퍼 재사용 (malloc 불필요)          │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 4ms: C++ 필터 실행 (WASM)                   │
│      - applySepia/applyXray/... (2~5ms)    │
│      ⚡ 필터에 따라 소요 시간 변동          │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 7ms: 결과 복사                              │
│      - subarray() + set() (0.5ms)           │
│      - 버퍼 재사용 (free 불필요)            │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 8ms: ctx.putImageData() + 성능 측정         │
│      - 처리된 이미지 → Canvas (0.8ms)       │
│      - processingTime, FPS 업데이트         │
└─────────────────┬───────────────────────────┘
                  │
┌─────────────────┴───────────────────────────┐
│ 9ms: requestAnimationFrame()                │
│      - 다음 프레임 예약                     │
└─────────────────────────────────────────────┘

총 소요 시간: ~9ms (16.67ms 예산의 54%)
여유 시간: ~7ms → 추가 필터나 효과 가능

성능 지표 (UI 색상 표시):
- 🟢 < 10ms (Excellent)
- 🟡 < 20ms (Good)
- 🟠 < 33ms (Warning)
- 🔴 ≥ 33ms (Critical)
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

✅ **현재 구현**:
```javascript
// Canvas 크기를 비디오 실제 크기에 맞춤
canvas.width = video.videoWidth;   // 640
canvas.height = video.videoHeight; // 480

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
| src/filters/filters.cpp | 필터 알고리즘 (sepia, xray, mirror 등) | C++ |
| web/app.js | 웹캠 제어, 필터 호출, WASM 로딩 | JavaScript |
| build/filters.wasm | 컴파일된 바이너리 | WebAssembly |
| build/filters.js | JS ↔ WASM 브릿지 (Emscripten 생성) | JavaScript |
| web/index.html | UI | HTML |
| web/styles.css | 디자인 | CSS |
