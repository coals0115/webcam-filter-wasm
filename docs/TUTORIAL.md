# 실습 튜토리얼

> 따라하면서 배우는 첫 번째 필터 만들기

## 목차
- [학습 목표](#학습-목표)
- [준비사항](#준비사항)
- [튜토리얼 1: 세피아 필터 만들기](#튜토리얼-1-세피아-필터-만들기)
- [튜토리얼 2: 밝기 조절 필터](#튜토리얼-2-밝기-조절-필터)
- [튜토리얼 3: 블러 효과 (도전 과제)](#튜토리얼-3-블러-효과-도전-과제)
- [디버깅 방법](#디버깅-방법)
- [다음 단계](#다음-단계)

---

## 학습 목표

이 튜토리얼을 완료하면:
- ✅ C++ 필터 함수를 직접 작성할 수 있습니다
- ✅ JavaScript에서 필터를 호출하는 방법을 이해합니다
- ✅ 픽셀 데이터를 다루는 방법을 배웁니다
- ✅ 빌드 → 테스트 → 디버깅 흐름을 익힙니다

---

## 준비사항

### 체크리스트

- [ ] 개발 환경 설정 완료 ([GETTING_STARTED.md](GETTING_STARTED.md) 참조)
- [ ] 프로젝트 빌드 성공 확인
- [ ] 웹캠이 정상 작동하는지 확인
- [ ] 텍스트 에디터 준비 (VS Code 권장)

### 필요한 사전 지식

- 기본 C++ 문법 (변수, 함수, 반복문)
- 기본 JavaScript 문법
- HTML/CSS 기초

---

## 튜토리얼 1: 세피아 필터 만들기

> 난이도: ⭐ 초급
> 예상 시간: 20분
> 목표: 이미지를 빈티지 세피아 톤으로 변환

### Step 1: C++ 필터 함수 작성

**파일**: `main.cpp`

기존 필터 함수 아래에 세피아 필터 함수를 추가합니다.

```cpp
/**
 * 세피아 필터 함수
 * @param ptr 메모리 포인터
 * @param length 배열 길이
 */
void applySepiaFilterRaw(uintptr_t ptr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);

    for (int i = 0; i < length; i += 4) {
        // 1. 원본 RGB 값 읽기
        uint8_t r = data[i];
        uint8_t g = data[i + 1];
        uint8_t b = data[i + 2];

        // 2. 세피아 변환 공식 적용
        uint8_t tr = static_cast<uint8_t>(
            std::min(255.0, (r * 0.393 + g * 0.769 + b * 0.189))
        );
        uint8_t tg = static_cast<uint8_t>(
            std::min(255.0, (r * 0.349 + g * 0.686 + b * 0.168))
        );
        uint8_t tb = static_cast<uint8_t>(
            std::min(255.0, (r * 0.272 + g * 0.534 + b * 0.131))
        );

        // 3. 변환된 값 저장
        data[i] = tr;
        data[i + 1] = tg;
        data[i + 2] = tb;
        // data[i + 3]는 alpha, 그대로 유지
    }
}
```

**코드 설명**:
- `std::min(255.0, ...)`: 값이 255를 넘지 않도록 제한
- 세피아 공식: 빈티지 사진 효과를 위한 표준 변환 공식
- `i += 4`: RGBA 4바이트씩 처리

### Step 2: JavaScript 바인딩 추가

같은 `main.cpp` 파일의 `EMSCRIPTEN_BINDINGS` 블록에 함수를 추가합니다.

```cpp
EMSCRIPTEN_BINDINGS(webcam_filter) {
    function("applyGrayscaleFilterRaw", &applyGrayscaleFilterRaw);
    function("applyHorizontalFlipRaw", &applyHorizontalFlipRaw);
    function("applySepiaFilterRaw", &applySepiaFilterRaw);  // 추가!
}
```

### Step 3: 헤더 추가

`main.cpp` 상단에 `<algorithm>` 헤더가 있는지 확인합니다.

```cpp
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cstdint>
#include <algorithm>  // std::min을 위해 필요
```

### Step 4: 빌드

터미널에서 빌드 스크립트를 실행합니다.

```bash
./build.sh
```

**예상 출력**:
```
cache:INFO: generating system asset: symbol_lists/77c73b…
cache:INFO: generating system library: compiler_rt.a... (this will be cached in …)
…빌드 진행 메시지…
✅ 빌드 완료: build/filters.js, build/filters.wasm
```

### Step 5: JavaScript 함수 작성

**파일**: `web/app.js`

기존 필터 함수들 아래에 세피아 필터 함수를 추가합니다.

```javascript
/**
 * 세피아 필터 적용
 */
function applySepiaFilter(imageData) {
    const bytes = imageData.data;
    const numBytes = bytes.length;

    // WASM 메모리 할당
    const ptr = Module._malloc(numBytes);

    try {
        // 데이터 복사 (JS → WASM)
        Module.HEAPU8.set(bytes, ptr);

        // C++ 함수 호출
        Module._applySepiaFilterRaw(ptr, numBytes);

        // 결과 가져오기 (WASM → JS)
        bytes.set(Module.HEAPU8.subarray(ptr, ptr + numBytes));
    } finally {
        // 메모리 해제 (필수!)
        Module._free(ptr);
    }
}
```

### Step 6: UI 버튼 추가

**파일**: `web/index.html`

필터 버튼 영역에 세피아 버튼을 추가합니다.

```html
<div class="button-group">
    <button id="noneBtn">원본</button>
    <button id="grayscaleBtn">흑백 필터</button>
    <button id="mirrorBtn">좌우반전</button>
    <button id="sepiaBtn">세피아</button>  <!-- 추가! -->
</div>
```

### Step 7: 이벤트 리스너 추가

**파일**: `web/app.js`

버튼 이벤트 리스너 영역에 추가합니다.

```javascript
// 버튼 이벤트 리스너 설정
document.getElementById('noneBtn')?.addEventListener('click', () => {
    currentFilter = 'none';
    setActiveButton('noneBtn');
});

document.getElementById('grayscaleBtn')?.addEventListener('click', () => {
    currentFilter = 'grayscale';
    setActiveButton('grayscaleBtn');
});

document.getElementById('mirrorBtn')?.addEventListener('click', () => {
    currentFilter = 'mirror';
    setActiveButton('mirrorBtn');
});

// 세피아 버튼 이벤트 추가!
document.getElementById('sepiaBtn')?.addEventListener('click', () => {
    currentFilter = 'sepia';
    setActiveButton('sepiaBtn');
});
```

### Step 8: 프레임 처리 함수 수정

**파일**: `web/app.js`

`processFrame()` 함수의 필터 분기에 세피아를 추가합니다.

```javascript
function processFrame() {
    if (!isProcessing) return;

    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);
    const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);

    // 필터 적용
    if (currentFilter === 'grayscale') {
        applyGrayscaleFilter(imageData);
    } else if (currentFilter === 'mirror') {
        applyMirrorFilter(imageData);
    } else if (currentFilter === 'sepia') {  // 추가!
        applySepiaFilter(imageData);
    }

    ctx.putImageData(imageData, 0, 0);
    requestAnimationFrame(processFrame);
}
```

### Step 9: 테스트

1. 개발 서버 실행:
```bash
./serve.sh
```

2. 브라우저 열기: `http://localhost:8000`

3. 세피아 버튼 클릭하여 테스트

**예상 결과**:
- 영상이 빈티지 세피아 톤으로 변환됨
- 따뜻한 갈색 톤의 이미지

### 완성! 🎉

축하합니다! 첫 번째 필터를 성공적으로 만들었습니다.

---

## 튜토리얼 2: 밝기 조절 필터

> 난이도: ⭐⭐ 중급
> 예상 시간: 30분
> 목표: 슬라이더로 밝기를 조절하는 필터

### Step 1: C++ 함수 작성

**파일**: `main.cpp`

```cpp
/**
 * 밝기 조절 필터
 * @param ptr 메모리 포인터
 * @param length 배열 길이
 * @param brightness 밝기 값 (-100 ~ 100)
 */
void applyBrightnessFilterRaw(uintptr_t ptr, int length, int brightness) {
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);

    for (int i = 0; i < length; i += 4) {
        // RGB 각 채널에 밝기 값 추가
        data[i] = std::clamp(data[i] + brightness, 0, 255);       // R
        data[i + 1] = std::clamp(data[i + 1] + brightness, 0, 255); // G
        data[i + 2] = std::clamp(data[i + 2] + brightness, 0, 255); // B
        // Alpha는 그대로
    }
}
```

**새로운 개념**:
- `std::clamp(value, min, max)`: 값을 min~max 범위로 제한
- `int brightness` 매개변수: JavaScript에서 전달받는 값

### Step 2: 바인딩 추가

```cpp
EMSCRIPTEN_BINDINGS(webcam_filter) {
    function("applyGrayscaleFilterRaw", &applyGrayscaleFilterRaw);
    function("applyHorizontalFlipRaw", &applyHorizontalFlipRaw);
    function("applySepiaFilterRaw", &applySepiaFilterRaw);
    function("applyBrightnessFilterRaw", &applyBrightnessFilterRaw);  // 추가!
}
```

### Step 3: 빌드

```bash
./build.sh
```

### Step 4: JavaScript 함수 작성

**파일**: `web/app.js`

```javascript
// 밝기 전역 변수
let brightnessValue = 0;

/**
 * 밝기 조절 필터
 */
function applyBrightnessFilter(imageData) {
    const bytes = imageData.data;
    const numBytes = bytes.length;
    const ptr = Module._malloc(numBytes);

    try {
        Module.HEAPU8.set(bytes, ptr);

        // C++ 함수 호출 (밝기 값 전달)
        Module._applyBrightnessFilterRaw(ptr, numBytes, brightnessValue);

        bytes.set(Module.HEAPU8.subarray(ptr, ptr + numBytes));
    } finally {
        Module._free(ptr);
    }
}
```

### Step 5: UI 슬라이더 추가

**파일**: `web/index.html`

```html
<div class="button-group">
    <button id="noneBtn">원본</button>
    <button id="grayscaleBtn">흑백 필터</button>
    <button id="mirrorBtn">좌우반전</button>
    <button id="sepiaBtn">세피아</button>
    <button id="brightnessBtn">밝기 조절</button>
</div>

<!-- 밝기 조절 슬라이더 추가 -->
<div id="brightnessControl" style="display: none; margin-top: 20px;">
    <label for="brightnessSlider">
        밝기: <span id="brightnessValue">0</span>
    </label>
    <input
        type="range"
        id="brightnessSlider"
        min="-100"
        max="100"
        value="0"
        style="width: 300px;"
    >
</div>
```

### Step 6: 슬라이더 이벤트 추가

**파일**: `web/app.js`

```javascript
// 밝기 버튼 이벤트
document.getElementById('brightnessBtn')?.addEventListener('click', () => {
    currentFilter = 'brightness';
    setActiveButton('brightnessBtn');

    // 슬라이더 표시
    const control = document.getElementById('brightnessControl');
    if (control) control.style.display = 'block';
});

// 슬라이더 이벤트
document.getElementById('brightnessSlider')?.addEventListener('input', (e) => {
    brightnessValue = parseInt(e.target.value);

    // 슬라이더 값 표시 업데이트
    const valueDisplay = document.getElementById('brightnessValue');
    if (valueDisplay) {
        valueDisplay.textContent = brightnessValue;
    }
});

// 다른 필터 선택 시 슬라이더 숨기기
function setActiveButton(buttonId) {
    // 기존 코드...

    // 밝기 조절이 아닌 경우 슬라이더 숨김
    if (buttonId !== 'brightnessBtn') {
        const control = document.getElementById('brightnessControl');
        if (control) control.style.display = 'none';
    }
}
```

### Step 7: 프레임 처리 추가

```javascript
function processFrame() {
    if (!isProcessing) return;

    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);
    const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);

    if (currentFilter === 'grayscale') {
        applyGrayscaleFilter(imageData);
    } else if (currentFilter === 'mirror') {
        applyMirrorFilter(imageData);
    } else if (currentFilter === 'sepia') {
        applySepiaFilter(imageData);
    } else if (currentFilter === 'brightness') {  // 추가!
        applyBrightnessFilter(imageData);
    }

    ctx.putImageData(imageData, 0, 0);
    requestAnimationFrame(processFrame);
}
```

### Step 8: 테스트

1. 서버 재시작 (이미 실행 중이면 새로고침)
2. "밝기 조절" 버튼 클릭
3. 슬라이더를 좌우로 움직여 밝기 변화 확인

**예상 결과**:
- 슬라이더를 오른쪽으로: 밝아짐
- 슬라이더를 왼쪽으로: 어두워짐

---

## 튜토리얼 3: 블러 효과 (도전 과제)

> 난이도: ⭐⭐⭐ 고급
> 예상 시간: 45분
> 목표: 가우시안 블러 효과 구현

### 개념 설명

블러 효과는 주변 픽셀의 평균값을 사용합니다.

```
원본 픽셀:           블러 적용:
[100, 150, 200]  →  주변 9픽셀의 평균
                    [(100+150+200+...)/9]
```

### Step 1: C++ 함수 작성 (심플 박스 블러)

**파일**: `main.cpp`

```cpp
/**
 * 심플 블러 필터 (3x3 박스 블러)
 * @param ptr 메모리 포인터
 * @param width 이미지 너비
 * @param height 이미지 높이
 */
void applyBlurFilterRaw(uintptr_t ptr, int width, int height) {
    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);

    // 임시 버퍼 생성 (결과 저장용)
    std::vector<uint8_t> temp(width * height * 4);

    // 전체 픽셀 순회
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int centerIndex = (y * width + x) * 4;

            int sumR = 0, sumG = 0, sumB = 0;
            int count = 0;

            // 주변 3x3 픽셀 순회
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;

                    // 경계 체크
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        int neighborIndex = (ny * width + nx) * 4;
                        sumR += data[neighborIndex];
                        sumG += data[neighborIndex + 1];
                        sumB += data[neighborIndex + 2];
                        count++;
                    }
                }
            }

            // 평균값 계산
            temp[centerIndex] = sumR / count;
            temp[centerIndex + 1] = sumG / count;
            temp[centerIndex + 2] = sumB / count;
            temp[centerIndex + 3] = data[centerIndex + 3];  // Alpha 유지
        }
    }

    // 결과를 원본 메모리로 복사
    std::copy(temp.begin(), temp.end(), data);
}
```

**추가 헤더**:
```cpp
#include <vector>  // std::vector 사용
```

### Step 2: 바인딩 및 빌드

```cpp
EMSCRIPTEN_BINDINGS(webcam_filter) {
    // ... 기존 함수들
    function("applyBlurFilterRaw", &applyBlurFilterRaw);
}
```

```bash
./build.sh
```

### Step 3: JavaScript 함수

**파일**: `web/app.js`

```javascript
function applyBlurFilter(imageData) {
    const bytes = imageData.data;
    const numBytes = bytes.length;
    const ptr = Module._malloc(numBytes);

    try {
        Module.HEAPU8.set(bytes, ptr);

        // width, height 전달 필요
        Module._applyBlurFilterRaw(ptr, canvas.width, canvas.height);

        bytes.set(Module.HEAPU8.subarray(ptr, ptr + numBytes));
    } finally {
        Module._free(ptr);
    }
}
```

### Step 4: UI 및 이벤트

```html
<button id="blurBtn">블러</button>
```

```javascript
document.getElementById('blurBtn')?.addEventListener('click', () => {
    currentFilter = 'blur';
    setActiveButton('blurBtn');
});

// processFrame()에 추가
else if (currentFilter === 'blur') {
    applyBlurFilter(imageData);
}
```

### 성능 주의사항

⚠️ **블러 필터는 연산량이 많습니다!**

- 3x3 블러: 각 픽셀마다 9번 계산
- 1920×1080 영상: 약 2백만 픽셀 × 9 = 1,800만 번 연산
- 60fps 유지가 어려울 수 있음

**최적화 방법**:
1. 해상도 낮추기: 640×480으로 처리
2. 프레임 건너뛰기: 2프레임마다 1번만 블러 적용
3. 블러 강도 줄이기: 3×3 대신 작은 영역만

---

## 디버깅 방법

### 1. 콘솔 로그 활용

```javascript
function applySepiaFilter(imageData) {
    console.log('Sepia filter called');
    console.log('Image size:', imageData.width, 'x', imageData.height);

    const bytes = imageData.data;
    console.log('First pixel RGBA:', bytes[0], bytes[1], bytes[2], bytes[3]);

    // ... 필터 처리

    console.log('After filter:', bytes[0], bytes[1], bytes[2], bytes[3]);
}
```

### 2. C++ 디버그 출력

```cpp
#include <emscripten/console.h>

void applySepiaFilterRaw(uintptr_t ptr, int length) {
    emscripten_console_log("Sepia filter C++ called");

    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);

    // 첫 픽셀 출력
    emscripten_console_logf("First pixel: R=%d G=%d B=%d",
        data[0], data[1], data[2]);

    // ... 처리
}
```

### 3. 브라우저 개발자 도구

```
F12 → Console 탭
- 에러 메시지 확인
- console.log 출력 확인

F12 → Performance 탭
- 프레임 속도 측정
- 함수 실행 시간 프로파일링
```

### 4. 일반적인 오류와 해결

#### 오류 1: 필터가 적용되지 않음

```javascript
// ❌ 잘못된 코드
Module._applySepiaFilter(ptr, numBytes);  // 함수명 오타

// ✅ 올바른 코드
Module._applySepiaFilterRaw(ptr, numBytes);
```

#### 오류 2: 화면이 깨짐

```cpp
// ❌ 잘못된 코드 - 범위 초과
data[i] = r * 2;  // 255 초과 가능

// ✅ 올바른 코드
data[i] = std::min(255, r * 2);
```

#### 오류 3: 메모리 누수

```javascript
// ❌ 잘못된 코드 - 메모리 해제 안 함
const ptr = Module._malloc(numBytes);
Module._applySepiaFilterRaw(ptr, numBytes);
// _free() 호출 안 함!

// ✅ 올바른 코드
const ptr = Module._malloc(numBytes);
try {
    Module._applySepiaFilterRaw(ptr, numBytes);
} finally {
    Module._free(ptr);  // 반드시 해제
}
```

---

## 성능 측정

### FPS 카운터 추가

**파일**: `web/app.js`

```javascript
let lastTime = performance.now();
let frameCount = 0;
let fps = 0;

function processFrame() {
    frameCount++;
    const now = performance.now();

    // 1초마다 FPS 계산
    if (now - lastTime >= 1000) {
        fps = frameCount;
        console.log('FPS:', fps);

        // 화면에 표시
        document.getElementById('fpsDisplay').textContent = `FPS: ${fps}`;

        frameCount = 0;
        lastTime = now;
    }

    // ... 기존 처리 코드
}
```

**HTML에 FPS 표시 추가**:
```html
<div id="fpsDisplay" style="position: fixed; top: 10px; right: 10px;
     background: rgba(0,0,0,0.7); color: white; padding: 10px;
     border-radius: 5px;">
    FPS: 0
</div>
```

---

## 도전 과제

### 1. 컬러 반전 필터
RGB 값을 반전시키는 필터를 만들어보세요.

**힌트**:
```cpp
// R, G, B 각각 반전
invertedR = 255 - R
invertedG = 255 - G
invertedB = 255 - B
```

### 2. 엣지 감지 필터
Sobel 필터를 사용한 엣지 감지를 구현해보세요.

**참고 자료**:
- [Sobel Filter Wikipedia](https://en.wikipedia.org/wiki/Sobel_operator)

### 3. 픽셀레이트 효과
이미지를 블록 단위로 모자이크 처리하는 필터

**힌트**:
```cpp
// 8x8 블록 단위로 평균값 계산
for (block_y = 0; block_y < height; block_y += 8) {
    for (block_x = 0; block_x < width; block_x += 8) {
        // 블록 내 평균 색상 계산
        // 블록 전체에 평균 색상 적용
    }
}
```

---

## 다음 단계

튜토리얼을 완료했다면:

1. 📚 [API 레퍼런스](API_REFERENCE.md) - 더 많은 함수와 옵션 탐색
2. 🔧 [문제 해결 가이드](TROUBLESHOOTING.md) - 자주 발생하는 문제 해결
3. 🚀 직접 필터 만들기 - 창의적인 필터 아이디어 구현

---

## 학습 체크리스트

- [ ] 세피아 필터 성공적으로 구현
- [ ] 밝기 조절 필터 완료 및 슬라이더 작동 확인
- [ ] 블러 필터 구현 (선택)
- [ ] JavaScript ↔ C++ 데이터 전달 방법 이해
- [ ] 메모리 할당/해제 패턴 숙지
- [ ] 빌드 → 테스트 → 디버깅 워크플로우 숙달
- [ ] FPS 모니터링 방법 습득

모든 항목을 체크했다면 축하합니다! 🎉
이제 WebAssembly 기반 이미지 처리의 기초를 마스터했습니다.

---

## 추가 학습 자료

### 이미지 처리 알고리즘
- [Computer Vision Algorithms](https://en.wikipedia.org/wiki/List_of_computer_vision_topics)
- [Image Processing Basics](https://opencv.org/)

### 성능 최적화
- [WebAssembly Performance Tips](https://emscripten.org/docs/optimizing/Optimizing-Code.html)
- [SIMD in WebAssembly](https://v8.dev/features/simd)

### 프로젝트 아이디어
1. 실시간 얼굴 감지 + 필터
2. AR 효과 (가상 선글라스, 모자 등)
3. 색상 팔레트 분석기
4. 실시간 그린스크린 제거
