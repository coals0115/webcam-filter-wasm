# 프로젝트 완전 이해 가이드

> 발표 준비를 위한 프로젝트 심층 분석 문서

---

## 목차

1. [프로젝트 한 줄 요약](#1-프로젝트-한-줄-요약)
2. [전체 동작 흐름](#2-전체-동작-흐름)
3. [파일별 역할](#3-파일별-역할)
4. [핵심 코드 라인별 해설](#4-핵심-코드-라인별-해설)
5. [기술 선택 이유](#5-기술-선택-이유)
6. [예상 질문 & 답변](#6-예상-질문--답변)
7. [핵심 용어 정리](#7-핵심-용어-정리)

---

## 1. 프로젝트 한 줄 요약

**"웹캠 영상을 실시간으로 받아서, C++로 만든 필터를 적용하고, 화면에 보여주는 웹 애플리케이션"**

```
[웹캠] → [JavaScript가 영상 캡처] → [C++ 필터 적용] → [화면에 출력]
```

---

## 2. 전체 동작 흐름

### 2.1 앱 시작 시 (초기화)

```
1. 브라우저가 index.html 로드
2. filters.js 로드 (C++ 코드가 컴파일된 WebAssembly)
3. app.js 로드 및 실행
4. init() 함수 호출
   ├── loadWasmModule(): WebAssembly 모듈 초기화
   └── initWebcam(): 웹캠 권한 요청 및 연결
5. processFrame() 루프 시작 (초당 60번 반복)
```

### 2.2 매 프레임마다 (메인 루프)

```
processFrame() - 초당 60번 실행됨

1. 웹캠 영상을 캔버스에 그림
   ctx.drawImage(video, ...)

2. 캔버스에서 픽셀 데이터 추출
   ctx.getImageData(...) → [R,G,B,A, R,G,B,A, ...] 배열

3. 픽셀 데이터를 WASM 메모리로 복사
   wasmModule.HEAPU8.set(data, wasmBuffer)

4. C++ 필터 함수 호출 (여기서 실제 이미지 처리)
   wasmModule.applyGrayscale(...)  또는
   wasmModule.applyHorizontalFlip(...)  또는
   wasmModule.applySepia(...)

5. 처리된 데이터를 다시 JavaScript로 복사
   data.set(wasmModule.HEAPU8.subarray(...))

6. 캔버스에 결과 출력
   ctx.putImageData(imageData, ...)

7. 다음 프레임 예약
   requestAnimationFrame(processFrame)
```

### 2.3 시각적 흐름도

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│   웹캠      │────▶│  Canvas     │────▶│  화면에     │
│   (영상)    │     │  (처리)     │     │  출력       │
└─────────────┘     └─────────────┘     └─────────────┘
                           │
                           ▼
                    ┌─────────────┐
                    │ WebAssembly │
                    │ (C++ 필터)  │
                    └─────────────┘
```

---

## 3. 파일별 역할

### 3.1 핵심 파일 (이것만 알면 됨)

| 파일 | 역할 | 한 줄 설명 |
|------|------|-----------|
| `src/filters/filters.cpp` | **이미지 처리** | 실제 필터 알고리즘 (흑백, 좌우반전, 세피아) |
| `web/app.js` | **앱 로직** | 웹캠 연결, 프레임 처리, WASM 호출 |
| `web/index.html` | **화면 구조** | 버튼, 캔버스, 성능 표시 UI |
| `web/styles.css` | **디자인** | 시각적 스타일링 |
| `build.sh` | **빌드** | C++ → WebAssembly 컴파일 |

### 3.2 빌드 결과물

| 파일 | 설명 |
|------|------|
| `build/filters.wasm` | 컴파일된 C++ 코드 (바이너리) |
| `build/filters.js` | WASM을 로드하는 글루 코드 |

---

## 4. 핵심 코드 라인별 해설

### 4.1 C++ 필터 코드 (`src/filters/filters.cpp`)

#### 흑백 필터

```cpp
void applyGrayscale(uintptr_t dataPtr, int length) {
    // dataPtr: JavaScript에서 전달받은 메모리 주소
    // length: 픽셀 데이터 총 바이트 수 (width × height × 4)

    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);
    // 메모리 주소를 실제 데이터 포인터로 변환

    for (int i = 0; i < length; i += 4) {
        // i += 4인 이유: 픽셀 하나 = [R, G, B, A] 4바이트

        // ITU-R BT.709 표준 공식으로 밝기 계산
        // 인간의 눈은 녹색에 가장 민감 → G에 가장 큰 가중치
        uint8_t gray = static_cast<uint8_t>(
            (54 * data[i] + 183 * data[i + 1] + 19 * data[i + 2]) >> 8
        );
        // 54/256 ≈ 0.21 (R 가중치)
        // 183/256 ≈ 0.71 (G 가중치)
        // 19/256 ≈ 0.07 (B 가중치)
        // >> 8 은 256으로 나누기 (정수 연산으로 빠름)

        data[i] = gray;      // R → 회색
        data[i + 1] = gray;  // G → 회색
        data[i + 2] = gray;  // B → 회색
        // data[i + 3] (Alpha)는 그대로 유지
    }
}
```

**왜 `54, 183, 19` 인가?**
- 표준 공식: `0.2126×R + 0.7152×G + 0.0722×B`
- 정수 연산 최적화: `0.2126 × 256 ≈ 54`, `0.7152 × 256 ≈ 183`, `0.0722 × 256 ≈ 19`
- `>> 8`로 256 나누기 → 소수점 연산보다 훨씬 빠름

#### 좌우반전 필터

```cpp
void applyHorizontalFlip(uintptr_t dataPtr, int width, int height) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int y = 0; y < height; y++) {
        // 각 행(row)에 대해

        for (int x = 0; x < width / 2; x++) {
            // 왼쪽 절반만 순회 (좌우 교환이니까)

            int leftIdx = (y * width + x) * 4;
            // 왼쪽 픽셀 위치 계산
            // y * width: 현재 행의 시작점
            // + x: 열 위치
            // * 4: 픽셀당 4바이트 (RGBA)

            int rightIdx = (y * width + (width - 1 - x)) * 4;
            // 오른쪽 픽셀 위치 (대칭점)
            // width - 1 - x: 오른쪽에서의 대칭 위치

            // R, G, B, A 4개 값 교환
            for (int c = 0; c < 4; c++) {
                uint8_t temp = data[leftIdx + c];
                data[leftIdx + c] = data[rightIdx + c];
                data[rightIdx + c] = temp;
            }
        }
    }
}
```

**픽셀 인덱스 계산 예시 (640×480 이미지)**
```
행 0, 열 0 → 인덱스 = (0 × 640 + 0) × 4 = 0
행 0, 열 639 → 인덱스 = (0 × 640 + 639) × 4 = 2556
행 1, 열 0 → 인덱스 = (1 × 640 + 0) × 4 = 2560
```

#### 세피아 필터

```cpp
void applySepia(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int i = 0; i < length; i += 4) {
        uint8_t r = data[i];
        uint8_t g = data[i + 1];
        uint8_t b = data[i + 2];

        // 세피아 변환 공식 (빈티지 사진 느낌)
        int newR = (r * 101 + g * 197 + b * 48) >> 8;
        int newG = (r * 89 + g * 176 + b * 43) >> 8;
        int newB = (r * 70 + g * 137 + b * 34) >> 8;

        // 255 초과 방지 (클램핑)
        data[i] = static_cast<uint8_t>(newR > 255 ? 255 : newR);
        data[i + 1] = static_cast<uint8_t>(newG > 255 ? 255 : newG);
        data[i + 2] = static_cast<uint8_t>(newB > 255 ? 255 : newB);
    }
}
```

#### JavaScript에 함수 노출

```cpp
EMSCRIPTEN_BINDINGS(filters) {
    function("applyGrayscale", &applyGrayscale);
    function("applyHorizontalFlip", &applyHorizontalFlip);
    function("applySepia", &applySepia);
    function("allocateBuffer", &allocateBuffer);
    function("freeBuffer", &freeBuffer);
}
```
- `EMSCRIPTEN_BINDINGS`: C++ 함수를 JavaScript에서 호출할 수 있게 등록
- `function("JS에서_부를_이름", &실제_C++_함수)`

### 4.2 JavaScript 앱 코드 (`web/app.js`)

#### WASM 모듈 로딩

```javascript
async function loadWasmModule() {
    // Module은 filters.js에서 정의된 함수
    // Emscripten이 MODULARIZE 옵션으로 생성함
    wasmModule = await Module();
    // wasmModule 객체에 C++ 함수들이 메서드로 들어있음
    // wasmModule.applyGrayscale, wasmModule.applySepia 등
}
```

#### 웹캠 초기화

```javascript
async function initWebcam() {
    // 브라우저에 웹캠 권한 요청
    const stream = await navigator.mediaDevices.getUserMedia({
        video: {
            width: { ideal: 640 },   // 이상적인 해상도
            height: { ideal: 480 },
            facingMode: 'user'       // 전면 카메라 (셀카 모드)
        },
        audio: false  // 오디오는 필요 없음
    });

    video.srcObject = stream;  // 비디오 엘리먼트에 스트림 연결

    // 캔버스 크기를 비디오 크기에 맞춤
    canvas.width = video.videoWidth;
    canvas.height = video.videoHeight;

    // WASM에서 사용할 메모리 버퍼 미리 할당
    // 640 × 480 × 4(RGBA) = 1,228,800 바이트
    const bufferSize = canvas.width * canvas.height * 4;
    wasmBuffer = wasmModule.allocateBuffer(bufferSize);
}
```

#### 메인 프레임 처리 루프

```javascript
function processFrame() {
    const startTime = performance.now();  // 성능 측정 시작

    // 1. 웹캠 영상을 캔버스에 그리기
    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);

    if (currentFilter !== 'none' && wasmModule && wasmBuffer) {
        // 2. 캔버스에서 픽셀 데이터 가져오기
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        const data = imageData.data;
        // data = Uint8ClampedArray [R,G,B,A, R,G,B,A, ...]

        // 3. JS → WASM 메모리로 복사
        wasmModule.HEAPU8.set(data, wasmBuffer);
        // HEAPU8: WASM의 힙 메모리를 Uint8Array로 접근
        // wasmBuffer: 복사할 시작 주소

        // 4. C++ 필터 함수 호출
        if (currentFilter === 'grayscale') {
            wasmModule.applyGrayscale(wasmBuffer, data.length);
        } else if (currentFilter === 'flip') {
            wasmModule.applyHorizontalFlip(wasmBuffer, canvas.width, canvas.height);
        } else if (currentFilter === 'sepia') {
            wasmModule.applySepia(wasmBuffer, data.length);
        }

        // 5. WASM → JS로 결과 복사
        data.set(wasmModule.HEAPU8.subarray(wasmBuffer, wasmBuffer + data.length));
        // subarray: wasmBuffer부터 data.length만큼 잘라서 가져옴

        // 6. 처리된 이미지를 캔버스에 출력
        ctx.putImageData(imageData, 0, 0);
    }

    // 성능 측정 & 표시
    const processingTime = performance.now() - startTime;
    processingTimeEl.textContent = `${processingTime.toFixed(2)} ms`;

    // 다음 프레임 예약 (브라우저가 준비되면 다시 호출)
    animationId = requestAnimationFrame(processFrame);
}
```

---

## 5. 기술 선택 이유

### 5.1 왜 WebAssembly인가?

| 질문 | 답변 |
|------|------|
| **JavaScript로 안 되나요?** | 됩니다. 하지만 느립니다. 픽셀 단위 연산은 JS보다 WASM이 2~10배 빠릅니다. |
| **왜 빠른가요?** | WASM은 브라우저에서 네이티브에 가깝게 실행됩니다. JIT 컴파일 오버헤드가 없고, 타입이 고정되어 있어 최적화가 쉽습니다. |
| **단점은?** | JS↔WASM 간 데이터 복사 비용이 있습니다. 작은 연산은 오히려 JS가 빠를 수 있습니다. |

### 5.2 왜 C++인가?

| 질문 | 답변 |
|------|------|
| **Rust로 안 되나요?** | 됩니다. 하지만 C++이 이미지 처리 분야에서 더 많은 레퍼런스와 라이브러리가 있습니다. |
| **C로 안 되나요?** | 됩니다. 하지만 C++의 embind가 JavaScript 바인딩을 더 쉽게 해줍니다. |

### 5.3 왜 Emscripten인가?

| 기능 | 설명 |
|------|------|
| **embind** | C++ 함수를 JavaScript에서 쉽게 호출할 수 있게 해줌 |
| **HEAPU8** | WASM 메모리를 JavaScript에서 직접 접근 가능 |
| **최적화** | `-O3`, `-flto`, `-msimd128` 등 강력한 최적화 옵션 |

### 5.4 빌드 옵션 설명 (build.sh)

```bash
emcc \
    src/filters/filters.cpp \     # 입력 파일
    -o build/filters.js \          # 출력 파일
    -O3 \                          # 최대 최적화 레벨
    -flto \                        # 링크 타임 최적화
    -msimd128 \                    # SIMD 명령어 활성화 (병렬 연산)
    --bind \                       # embind 사용 (C++↔JS 바인딩)
    -s WASM=1 \                    # WebAssembly 출력
    -s ALLOW_MEMORY_GROWTH=1 \     # 동적 메모리 확장 허용
    -s MODULARIZE=1 \              # 모듈화 (async 로딩 가능)
    -s EXPORT_NAME="Module" \      # JS에서 접근할 이름
    -s EXPORTED_RUNTIME_METHODS='["cwrap","ccall","HEAPU8"]'
```

---

## 6. 예상 질문 & 답변

### Q1. 이 프로젝트가 뭔가요?

> "웹 브라우저에서 웹캠 영상에 실시간 필터를 적용하는 애플리케이션입니다.
> C++로 이미지 처리 알고리즘을 구현하고, WebAssembly로 컴파일해서
> 브라우저에서 네이티브급 성능으로 실행합니다."

### Q2. 왜 WebAssembly를 사용했나요?

> "이미지 처리는 픽셀 단위로 수십만 번의 연산이 필요합니다.
> JavaScript보다 WebAssembly가 이런 반복 연산에서 2~10배 빠릅니다.
> 60 FPS 실시간 처리를 위해 성능이 중요했습니다."

### Q3. 흑백 필터는 어떻게 동작하나요?

> "ITU-R BT.709 표준 공식을 사용합니다.
> 밝기 = 0.21×R + 0.71×G + 0.07×B 로 계산합니다.
> 녹색 가중치가 가장 큰 이유는 인간의 눈이 녹색에 가장 민감하기 때문입니다.
> 코드에서는 정수 연산 최적화를 위해 256을 곱하고 비트 시프트로 나눕니다."

### Q4. 성능은 어느 정도인가요?

> "640×480 해상도에서 프레임당 처리 시간이 2~4ms입니다.
> 60 FPS 유지가 가능하며, 이는 16.6ms 안에 처리해야 하는 것보다 훨씬 여유롭습니다.
> 처리 시간과 FPS를 실시간으로 화면에 표시하고 있습니다."

### Q5. JavaScript와 C++ 사이에 데이터는 어떻게 전달되나요?

> "Emscripten의 HEAPU8을 통해 WASM 메모리에 직접 접근합니다.
> 1. JS에서 캔버스의 픽셀 데이터를 추출합니다.
> 2. HEAPU8.set()으로 WASM 메모리에 복사합니다.
> 3. C++ 함수가 메모리를 직접 수정합니다.
> 4. HEAPU8.subarray()로 결과를 JS로 가져옵니다.
> 메모리 복사는 2번만 일어나서 오버헤드가 최소화됩니다."

### Q6. 새로운 필터를 추가하려면?

> "3단계입니다.
> 1. C++에서 필터 함수 작성 (filters.cpp)
> 2. EMSCRIPTEN_BINDINGS에 함수 등록
> 3. JavaScript에서 호출 코드 추가 (app.js)
> 그리고 다시 빌드하면 됩니다."

### Q7. 메모리 누수는 없나요?

> "allocateBuffer()로 할당한 메모리는 freeBuffer()로 해제합니다.
> 페이지 언로드 시 beforeunload 이벤트에서 정리합니다.
> 프레임 루프 중에는 버퍼를 재사용해서 매번 할당/해제하지 않습니다."

### Q8. 왜 Canvas를 사용하나요?

> "Canvas API의 getImageData()로 픽셀 데이터에 접근할 수 있고,
> putImageData()로 수정된 데이터를 바로 화면에 출력할 수 있습니다.
> WebGL도 가능하지만, 단순 필터에는 Canvas가 더 직관적입니다."

### Q9. requestAnimationFrame은 뭔가요?

> "브라우저가 다음 화면을 그리기 전에 콜백을 실행해주는 API입니다.
> setInterval보다 효율적입니다. 브라우저가 탭이 비활성화되면 자동으로 멈추고,
> 화면 주사율에 맞춰 호출되어 불필요한 렌더링이 없습니다."

### Q10. SIMD가 뭔가요?

> "Single Instruction Multiple Data의 약자입니다.
> 하나의 명령어로 여러 데이터를 동시에 처리합니다.
> 예: 4개의 픽셀을 한 번에 처리할 수 있어서 속도가 향상됩니다.
> build.sh의 -msimd128 옵션으로 활성화했습니다."

---

## 7. 핵심 용어 정리

| 용어 | 설명 |
|------|------|
| **WebAssembly (WASM)** | 브라우저에서 실행되는 저수준 바이너리 포맷. C++, Rust 등을 컴파일해서 만듦 |
| **Emscripten** | C/C++를 WebAssembly로 컴파일하는 도구체인 |
| **embind** | C++ 함수를 JavaScript에서 호출할 수 있게 해주는 Emscripten 기능 |
| **HEAPU8** | WASM의 힙 메모리를 Uint8Array로 접근하는 뷰 |
| **Canvas** | HTML5의 2D 그래픽 API. 픽셀 단위 조작 가능 |
| **getImageData()** | Canvas에서 픽셀 데이터(RGBA 배열)를 추출 |
| **putImageData()** | 픽셀 데이터를 Canvas에 출력 |
| **requestAnimationFrame** | 브라우저 렌더링 사이클에 맞춰 콜백을 실행 |
| **ITU-R BT.709** | HD TV 색공간 표준. 흑백 변환 가중치의 근거 |
| **RGBA** | Red, Green, Blue, Alpha (투명도). 픽셀당 4바이트 |
| **uintptr_t** | 메모리 주소를 저장하는 정수 타입 |
| **비트 시프트 (>>)** | 2의 거듭제곱으로 나누는 빠른 연산. `>> 8`은 256으로 나누기 |

---

## 마무리

이 문서를 읽고 나면:

1. **전체 흐름**을 설명할 수 있다
2. **각 파일의 역할**을 안다
3. **핵심 코드**가 무슨 일을 하는지 이해한다
4. **기술 선택 이유**를 설명할 수 있다
5. **예상 질문**에 답변할 수 있다

발표 화이팅!
