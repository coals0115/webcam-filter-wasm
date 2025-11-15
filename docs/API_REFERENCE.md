# API 레퍼런스

> WebAssembly 필터 함수와 JavaScript 인터페이스 상세 설명

## 목차
- [C++ API](#c-api)
  - [필터 함수](#필터-함수)
  - [데이터 타입](#데이터-타입)
  - [유틸리티 함수](#유틸리티-함수)
- [JavaScript API](#javascript-api)
  - [Module 객체](#module-객체)
  - [메모리 관리](#메모리-관리)
  - [필터 래퍼 함수](#필터-래퍼-함수)
- [Emscripten 바인딩](#emscripten-바인딩)
- [타입 변환](#타입-변환)
- [에러 처리](#에러-처리)

---

## C++ API

### 필터 함수

#### applyGrayscaleFilterRaw

이미지를 흑백으로 변환합니다.

**시그니처**:
```cpp
void applyGrayscaleFilterRaw(uintptr_t ptr, int length)
```

**매개변수**:
- `ptr` (uintptr_t): 이미지 데이터 메모리 주소
- `length` (int): 배열 길이 (바이트 단위, width × height × 4)

**알고리즘**:
ITU-R BT.709 표준 그레이스케일 변환 공식
```
Gray = 0.2126 * R + 0.7152 * G + 0.0722 * B
```

**예제**:
```cpp
// C++에서 직접 호출 (테스트용)
uint8_t imageData[1920 * 1080 * 4];  // RGBA 배열
applyGrayscaleFilterRaw(
    reinterpret_cast<uintptr_t>(imageData),
    1920 * 1080 * 4
);
```

**성능**:
- 시간 복잡도: O(n) - n은 픽셀 수
- 공간 복잡도: O(1) - 제자리 수정
- 1920×1080 @ 60fps: ~3-5ms per frame

**주의사항**:
- Alpha 채널(4번째 바이트)은 수정하지 않음
- 메모리는 4의 배수 길이여야 함 (RGBA 형식)

---

#### applyHorizontalFlipRaw

이미지를 좌우반전합니다.

**시그니처**:
```cpp
void applyHorizontalFlipRaw(uintptr_t ptr, int width, int height)
```

**매개변수**:
- `ptr` (uintptr_t): 이미지 데이터 메모리 주소
- `width` (int): 이미지 너비 (픽셀 단위)
- `height` (int): 이미지 높이 (픽셀 단위)

**알고리즘**:
```
각 행(row)마다:
  for x in [0, width/2):
    swap(pixel[x], pixel[width - 1 - x])
```

**예제**:
```cpp
uint8_t imageData[1920 * 1080 * 4];
applyHorizontalFlipRaw(
    reinterpret_cast<uintptr_t>(imageData),
    1920,  // width
    1080   // height
);
```

**성능**:
- 시간 복잡도: O(n) - n은 픽셀 수
- 공간 복잡도: O(1) - 제자리 교환
- 1920×1080 @ 60fps: ~2-4ms per frame

**주의사항**:
- RGBA 4채널 모두 교환됨
- width × height × 4 = 실제 배열 길이

---

### 데이터 타입

#### 픽셀 데이터 구조

```cpp
// RGBA 픽셀 표현
struct Pixel {
    uint8_t r;  // Red   (0-255)
    uint8_t g;  // Green (0-255)
    uint8_t b;  // Blue  (0-255)
    uint8_t a;  // Alpha (0-255, 255 = 불투명)
};

// 메모리 레이아웃
// [R0, G0, B0, A0, R1, G1, B1, A1, R2, G2, B2, A2, ...]
//  ↑ Pixel 0    ↑ Pixel 1      ↑ Pixel 2
```

#### 메모리 주소 타입

```cpp
// uintptr_t: 포인터를 정수로 표현
uintptr_t ptr = reinterpret_cast<uintptr_t>(data);

// JavaScript에서 전달받을 때 사용
// 64비트 시스템에서도 안전한 포인터 표현
```

---

### 유틸리티 함수

#### 픽셀 인덱스 계산

```cpp
/**
 * 2D 좌표를 1D 배열 인덱스로 변환
 */
inline int getPixelIndex(int x, int y, int width) {
    return (y * width + x) * 4;
}

// 사용 예:
int index = getPixelIndex(100, 200, 1920);
uint8_t red = data[index];
uint8_t green = data[index + 1];
uint8_t blue = data[index + 2];
uint8_t alpha = data[index + 3];
```

#### 색상 값 제한

```cpp
/**
 * 값을 0-255 범위로 제한
 */
inline uint8_t clampColor(int value) {
    return static_cast<uint8_t>(std::clamp(value, 0, 255));
}

// 사용 예:
int brightened = red + 50;
data[index] = clampColor(brightened);
```

#### RGB ↔ HSV 변환

```cpp
/**
 * RGB를 HSV로 변환
 */
struct HSV {
    float h;  // Hue (0-360)
    float s;  // Saturation (0-1)
    float v;  // Value (0-1)
};

HSV rgbToHSV(uint8_t r, uint8_t g, uint8_t b) {
    float rf = r / 255.0f;
    float gf = g / 255.0f;
    float bf = b / 255.0f;

    float max = std::max({rf, gf, bf});
    float min = std::min({rf, gf, bf});
    float delta = max - min;

    HSV hsv;
    hsv.v = max;
    hsv.s = (max == 0) ? 0 : delta / max;

    if (delta == 0) {
        hsv.h = 0;
    } else if (max == rf) {
        hsv.h = 60 * fmod((gf - bf) / delta, 6);
    } else if (max == gf) {
        hsv.h = 60 * ((bf - rf) / delta + 2);
    } else {
        hsv.h = 60 * ((rf - gf) / delta + 4);
    }

    if (hsv.h < 0) hsv.h += 360;
    return hsv;
}
```

---

## JavaScript API

### Module 객체

Emscripten이 생성하는 전역 객체입니다.

#### Module 속성

```javascript
// Module 객체 구조
const Module = {
    // WASM 메모리 뷰
    HEAPU8: Uint8Array,      // 8비트 unsigned 뷰
    HEAP8: Int8Array,        // 8비트 signed 뷰
    HEAPU16: Uint16Array,    // 16비트 unsigned 뷰
    HEAP16: Int16Array,      // 16비트 signed 뷰
    HEAPU32: Uint32Array,    // 32비트 unsigned 뷰
    HEAP32: Int32Array,      // 32비트 signed 뷰
    HEAPF32: Float32Array,   // 32비트 float 뷰
    HEAPF64: Float64Array,   // 64비트 float 뷰

    // 메모리 관리 함수
    _malloc: Function,       // 메모리 할당
    _free: Function,         // 메모리 해제

    // C++ 바인딩 함수 (언더스코어 prefix)
    _applyGrayscaleFilterRaw: Function,
    _applyHorizontalFlipRaw: Function,

    // 초기화 콜백
    onRuntimeInitialized: Function,

    // 기타 Emscripten 함수들...
};
```

---

### 메모리 관리

#### _malloc(size)

WASM 힙 메모리를 할당합니다.

**시그니처**:
```javascript
Module._malloc(size: number): number
```

**매개변수**:
- `size` (number): 할당할 바이트 수

**반환값**:
- (number): 할당된 메모리의 주소 (포인터)
- 실패 시: 0 (null pointer)

**예제**:
```javascript
const numBytes = 1920 * 1080 * 4;
const ptr = Module._malloc(numBytes);

if (ptr === 0) {
    console.error('메모리 할당 실패!');
    return;
}

// 메모리 사용...

Module._free(ptr);  // 반드시 해제
```

**주의사항**:
- C의 `malloc()`과 동일하게 동작
- 할당 후 **반드시** `_free()` 호출 필요
- 메모리 누수 방지를 위해 `try-finally` 사용 권장

---

#### _free(ptr)

할당된 WASM 메모리를 해제합니다.

**시그니처**:
```javascript
Module._free(ptr: number): void
```

**매개변수**:
- `ptr` (number): 해제할 메모리 주소

**예제**:
```javascript
const ptr = Module._malloc(1024);

try {
    // 메모리 사용
    Module.HEAPU8.set(data, ptr);
} finally {
    // 반드시 해제
    Module._free(ptr);
}
```

**주의사항**:
- 같은 포인터를 두 번 해제하면 안 됨 (double free)
- 이미 해제된 메모리에 접근하면 안 됨 (use after free)

---

#### HEAPU8.set(array, offset)

JavaScript 배열을 WASM 메모리로 복사합니다.

**시그니처**:
```javascript
Module.HEAPU8.set(
    source: Uint8ClampedArray | Uint8Array,
    offset: number
): void
```

**매개변수**:
- `source`: 복사할 원본 배열
- `offset`: 복사 시작 위치 (메모리 주소)

**예제**:
```javascript
const imageData = ctx.getImageData(0, 0, width, height);
const bytes = imageData.data;  // Uint8ClampedArray
const ptr = Module._malloc(bytes.length);

// JavaScript → WASM 메모리 복사
Module.HEAPU8.set(bytes, ptr);
```

---

#### HEAPU8.subarray(begin, end)

WASM 메모리의 특정 영역을 Uint8Array로 참조합니다.

**시그니처**:
```javascript
Module.HEAPU8.subarray(
    begin: number,
    end: number
): Uint8Array
```

**매개변수**:
- `begin`: 시작 주소
- `end`: 종료 주소 (미포함)

**반환값**:
- Uint8Array: 메모리 영역을 참조하는 배열 (복사 아님!)

**예제**:
```javascript
const ptr = Module._malloc(1000);

// WASM 메모리 → JavaScript 배열 참조
const view = Module.HEAPU8.subarray(ptr, ptr + 1000);

// 데이터 복사
const copy = new Uint8Array(view);

Module._free(ptr);
```

**주의사항**:
- `subarray()`는 복사가 아닌 참조
- 실제 복사가 필요하면 `new Uint8Array(subarray)`

---

### 필터 래퍼 함수

#### applyGrayscaleFilter(imageData)

흑백 필터를 적용합니다.

**시그니처**:
```javascript
function applyGrayscaleFilter(imageData: ImageData): void
```

**매개변수**:
- `imageData` (ImageData): Canvas ImageData 객체

**예제**:
```javascript
const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);

// 필터 적용
applyGrayscaleFilter(imageData);

// 결과를 Canvas에 표시
ctx.putImageData(imageData, 0, 0);
```

**내부 구현**:
```javascript
function applyGrayscaleFilter(imageData) {
    const bytes = imageData.data;
    const numBytes = bytes.length;
    const ptr = Module._malloc(numBytes);

    try {
        // JS → WASM
        Module.HEAPU8.set(bytes, ptr);

        // C++ 함수 호출
        Module._applyGrayscaleFilterRaw(ptr, numBytes);

        // WASM → JS
        bytes.set(Module.HEAPU8.subarray(ptr, ptr + numBytes));
    } finally {
        Module._free(ptr);
    }
}
```

**성능**:
- 메모리 할당/해제: ~0.5ms
- 데이터 복사: ~1ms
- C++ 처리: ~3-5ms
- 총: ~5-7ms per frame

---

#### applyMirrorFilter(imageData)

좌우반전 필터를 적용합니다.

**시그니처**:
```javascript
function applyMirrorFilter(imageData: ImageData): void
```

**매개변수**:
- `imageData` (ImageData): Canvas ImageData 객체

**예제**:
```javascript
const imageData = ctx.getImageData(0, 0, width, height);
applyMirrorFilter(imageData);
ctx.putImageData(imageData, 0, 0);
```

**내부 구현**:
```javascript
function applyMirrorFilter(imageData) {
    const bytes = imageData.data;
    const numBytes = bytes.length;
    const ptr = Module._malloc(numBytes);

    try {
        Module.HEAPU8.set(bytes, ptr);

        // width, height 전달 필요
        Module._applyHorizontalFlipRaw(
            ptr,
            imageData.width,
            imageData.height
        );

        bytes.set(Module.HEAPU8.subarray(ptr, ptr + numBytes));
    } finally {
        Module._free(ptr);
    }
}
```

---

## Emscripten 바인딩

### EMSCRIPTEN_BINDINGS 매크로

C++ 함수를 JavaScript에 노출합니다.

**기본 사용법**:
```cpp
#include <emscripten/bind.h>

using namespace emscripten;

// C++ 함수
void myFunction(int x, float y) {
    // ...
}

// JavaScript 바인딩
EMSCRIPTEN_BINDINGS(my_module) {
    function("myFunction", &myFunction);
}
```

**JavaScript에서 호출**:
```javascript
Module._myFunction(42, 3.14);
```

---

### function() 바인딩

함수를 바인딩합니다.

**시그니처**:
```cpp
function(
    const char* name,        // JavaScript 함수 이름
    FunctionType* function,  // C++ 함수 포인터
    allow_raw_pointers<>()   // 원시 포인터 허용 (선택)
)
```

**예제**:
```cpp
void procesImage(uintptr_t ptr, int size);

EMSCRIPTEN_BINDINGS(image_processing) {
    function("processImage", &processImage);
}
```

---

### 여러 함수 바인딩

```cpp
EMSCRIPTEN_BINDINGS(filters) {
    function("grayscale", &applyGrayscaleFilterRaw);
    function("mirror", &applyHorizontalFlipRaw);
    function("sepia", &applySepiaFilterRaw);
    function("blur", &applyBlurFilterRaw);
}
```

---

## 타입 변환

### C++ → JavaScript

| C++ 타입 | JavaScript 타입 | 예제 |
|----------|----------------|------|
| `int` | `number` | `42` |
| `float` | `number` | `3.14` |
| `double` | `number` | `3.14159` |
| `bool` | `boolean` | `true` |
| `std::string` | `string` | `"hello"` |
| `uintptr_t` | `number` | `12345678` |
| `uint8_t*` | (주소만 전달) | `ptr` |

---

### JavaScript → C++

```javascript
// JavaScript
Module._myFunction(
    42,           // int
    3.14,         // float
    true,         // bool
    ptr,          // uintptr_t (메모리 주소)
    "hello"       // std::string (자동 변환)
);
```

```cpp
// C++
void myFunction(
    int x,
    float y,
    bool flag,
    uintptr_t ptr,
    std::string str
) {
    // ...
}
```

---

## 에러 처리

### JavaScript에서 WASM 에러 처리

```javascript
function safeApplyFilter(imageData, filterFunc) {
    let ptr = 0;

    try {
        const bytes = imageData.data;
        const numBytes = bytes.length;

        // 메모리 할당
        ptr = Module._malloc(numBytes);
        if (ptr === 0) {
            throw new Error('WASM 메모리 할당 실패');
        }

        // 데이터 복사
        Module.HEAPU8.set(bytes, ptr);

        // 필터 실행
        filterFunc(ptr, numBytes);

        // 결과 가져오기
        bytes.set(Module.HEAPU8.subarray(ptr, ptr + numBytes));

    } catch (error) {
        console.error('필터 적용 실패:', error);
        throw error;

    } finally {
        // 메모리 해제 (에러 발생 시에도)
        if (ptr !== 0) {
            Module._free(ptr);
        }
    }
}
```

---

### C++에서 에러 처리

```cpp
#include <emscripten/console.h>

void applyFilterSafe(uintptr_t ptr, int length) {
    // 입력 검증
    if (ptr == 0) {
        emscripten_console_error("Invalid pointer: null");
        return;
    }

    if (length <= 0 || length % 4 != 0) {
        emscripten_console_error("Invalid length: not multiple of 4");
        return;
    }

    // 처리...
}
```

---

## 성능 최적화 팁

### 1. 메모리 재사용

```javascript
class FilterProcessor {
    constructor(width, height) {
        this.width = width;
        this.height = height;
        this.numBytes = width * height * 4;

        // 메모리 미리 할당
        this.ptr = Module._malloc(this.numBytes);
    }

    applyFilter(imageData, filterFunc) {
        // 재사용: 할당/해제 오버헤드 제거
        Module.HEAPU8.set(imageData.data, this.ptr);
        filterFunc(this.ptr, this.numBytes);
        imageData.data.set(
            Module.HEAPU8.subarray(this.ptr, this.ptr + this.numBytes)
        );
    }

    destroy() {
        Module._free(this.ptr);
    }
}

// 사용
const processor = new FilterProcessor(1920, 1080);
processor.applyFilter(imageData, Module._applyGrayscaleFilterRaw);
// ...
processor.destroy();  // 종료 시
```

### 2. 배치 처리

```javascript
// ❌ 비효율적: 여러 필터를 순차 적용
applyGrayscaleFilter(imageData);  // malloc → process → free
applySepiaFilter(imageData);      // malloc → process → free

// ✅ 효율적: 한 번에 여러 필터 적용
function applyMultipleFilters(imageData, filters) {
    const ptr = Module._malloc(imageData.data.length);

    try {
        Module.HEAPU8.set(imageData.data, ptr);

        // 여러 필터 순차 적용 (메모리 재사용)
        for (const filter of filters) {
            filter(ptr, imageData.data.length);
        }

        imageData.data.set(
            Module.HEAPU8.subarray(ptr, ptr + imageData.data.length)
        );
    } finally {
        Module._free(ptr);
    }
}
```

---

## 추가 리소스

### Emscripten 문서
- [Embind (C++ ↔ JavaScript 바인딩)](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html)
- [메모리 관리](https://emscripten.org/docs/porting/emscripten-runtime-environment.html)
- [타입 변환](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/WebIDL-Binder.html)

### WebAssembly 표준
- [WebAssembly Specification](https://webassembly.github.io/spec/)
- [JavaScript API](https://developer.mozilla.org/en-US/docs/WebAssembly/JavaScript_interface)

---

## 빠른 참조

### 필수 패턴

```javascript
// 1. 메모리 할당 → 처리 → 해제 패턴
const ptr = Module._malloc(size);
try {
    Module.HEAPU8.set(data, ptr);
    Module._filterFunction(ptr, size);
    data.set(Module.HEAPU8.subarray(ptr, ptr + size));
} finally {
    Module._free(ptr);
}

// 2. 초기화 대기 패턴
Module.onRuntimeInitialized = function() {
    console.log('WASM 준비 완료');
    initApp();
};

// 3. 에러 처리 패턴
if (ptr === 0) {
    throw new Error('메모리 할당 실패');
}
```

### 일반적인 실수

```javascript
// ❌ 잘못된 예
const ptr = Module._malloc(1000);
Module._filterFunction(ptr, 1000);
// _free() 호출 안 함! → 메모리 누수

// ✅ 올바른 예
const ptr = Module._malloc(1000);
try {
    Module._filterFunction(ptr, 1000);
} finally {
    Module._free(ptr);
}
```

---

## 다음 단계

- 🔧 [문제 해결 가이드](TROUBLESHOOTING.md) - 자주 발생하는 문제
- 🎓 [튜토리얼](TUTORIAL.md) - 실습 가이드
- 📖 [아키텍처](ARCHITECTURE.md) - 프로젝트 구조
