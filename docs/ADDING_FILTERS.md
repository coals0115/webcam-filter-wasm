# 새 필터 추가 가이드

이 문서는 WebCam Filter WASM 프로젝트에 새로운 필터를 추가하는 방법을 설명합니다.

## 개요

필터를 추가하려면 **4개 파일**을 수정해야 합니다:

| 순서 | 파일 | 작업 내용 |
|------|------|-----------|
| 1 | `src/filters/filters.cpp` | C++ 필터 함수 작성 + WASM 바인딩 |
| 2 | `web/index.html` | 버튼 UI 추가 |
| 3 | `web/app.js` | 버튼 참조 + 이벤트 리스너 + 필터 호출 |
| 4 | 빌드 | `./build.sh` 실행 |

> **주의**: 하나라도 누락하면 필터가 동작하지 않습니다!

---

## Step 1: C++ 필터 함수 작성

### 파일: `src/filters/filters.cpp`

### 1-1. 필터 함수 추가

`EMSCRIPTEN_BINDINGS` 블록 **위에** 함수를 작성합니다.

```cpp
/**
 * 반전(Invert) 필터 구현
 *
 * @param dataPtr 픽셀 데이터 포인터 (RGBA 형식)
 * @param length 데이터 길이 (바이트)
 */
void applyInvert(uintptr_t dataPtr, int length) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int i = 0; i < length; i += 4) {
        data[i] = 255 - data[i];         // R
        data[i + 1] = 255 - data[i + 1]; // G
        data[i + 2] = 255 - data[i + 2]; // B
        // Alpha (data[i + 3])는 그대로 유지
    }
}
```

### 1-2. WASM 바인딩 추가

`EMSCRIPTEN_BINDINGS` 블록 안에 함수를 등록합니다.

```cpp
EMSCRIPTEN_BINDINGS(filters) {
    function("applyGrayscale", &applyGrayscale);
    function("applyHorizontalFlip", &applyHorizontalFlip);
    function("applySepia", &applySepia);
    function("applyInvert", &applyInvert);  // <- 추가
    function("allocateBuffer", &allocateBuffer);
    function("freeBuffer", &freeBuffer);
}
```

### 필터 함수 시그니처

필터 유형에 따라 두 가지 시그니처를 사용합니다:

```cpp
// 타입 1: 픽셀 단위 처리 (흑백, 세피아, 반전 등)
void applyFilterName(uintptr_t dataPtr, int length);

// 타입 2: 좌표 기반 처리 (좌우반전, 회전 등)
void applyFilterName(uintptr_t dataPtr, int width, int height);
```

---

## Step 2: HTML 버튼 추가

### 파일: `web/index.html`

`filter-buttons` 내부에 버튼을 추가합니다.

```html
<nav class="filter-buttons" role="navigation" aria-label="필터 선택">
    <button id="btnNone" class="filter-btn active" aria-pressed="true" data-filter="none">
        원본
    </button>
    <button id="btnGrayscale" class="filter-btn" aria-pressed="false" data-filter="grayscale">
        흑백
    </button>
    <button id="btnSepia" class="filter-btn" aria-pressed="false" data-filter="sepia">
        세피아
    </button>
    <button id="btnFlip" class="filter-btn" aria-pressed="false" data-filter="flip">
        좌우반전
    </button>
    <!-- 새 필터 버튼 추가 -->
    <button id="btnInvert" class="filter-btn" aria-pressed="false" data-filter="invert">
        반전
    </button>
</nav>
```

### 필수 속성

| 속성 | 설명 |
|------|------|
| `id` | JavaScript에서 참조할 고유 ID (예: `btnInvert`) |
| `class="filter-btn"` | 스타일 적용을 위한 클래스 |
| `aria-pressed="false"` | 접근성 - 초기 상태는 false |
| `data-filter` | 필터 식별자 (예: `invert`) |

---

## Step 3: JavaScript 연동

### 파일: `web/app.js`

### 3-1. 버튼 참조 추가 (상단 DOM 요소 섹션)

```javascript
// 버튼
const btnNone = document.getElementById('btnNone');
const btnGrayscale = document.getElementById('btnGrayscale');
const btnFlip = document.getElementById('btnFlip');
const btnSepia = document.getElementById('btnSepia');
const btnInvert = document.getElementById('btnInvert');  // <- 추가
```

### 3-2. 이벤트 리스너 등록 (`setupEventListeners` 함수)

```javascript
function setupEventListeners() {
    btnNone.addEventListener('click', () => setFilter('none'));
    btnGrayscale.addEventListener('click', () => setFilter('grayscale'));
    btnFlip.addEventListener('click', () => setFilter('flip'));
    btnSepia.addEventListener('click', () => setFilter('sepia'));
    btnInvert.addEventListener('click', () => setFilter('invert'));  // <- 추가
}
```

### 3-3. 버튼 활성화 처리 (`setFilter` 함수)

```javascript
function setFilter(filter) {
    currentFilter = filter;

    // 모든 버튼에서 active 클래스 제거 및 aria-pressed 업데이트
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.classList.remove('active');
        btn.setAttribute('aria-pressed', 'false');
    });

    // 선택된 버튼에 active 클래스 추가 및 aria-pressed 업데이트
    let activeBtn;
    if (filter === 'none') {
        activeBtn = btnNone;
    } else if (filter === 'grayscale') {
        activeBtn = btnGrayscale;
    } else if (filter === 'flip') {
        activeBtn = btnFlip;
    } else if (filter === 'sepia') {
        activeBtn = btnSepia;
    } else if (filter === 'invert') {
        activeBtn = btnInvert;  // <- 추가
    }

    // ... 이하 생략
}
```

### 3-4. WASM 필터 호출 (`processFrame` 함수) - 가장 중요!

```javascript
function processFrame() {
    // ... 생략 ...

    // 필터 적용
    if (currentFilter !== 'none' && wasmModule && wasmBuffer) {
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        const data = imageData.data;

        // 1. JS 데이터를 WASM 메모리로 복사
        wasmModule.HEAPU8.set(data, wasmBuffer);

        // 2. WASM에서 필터 처리
        if (currentFilter === 'grayscale') {
            wasmModule.applyGrayscale(wasmBuffer, data.length);
        } else if (currentFilter === 'flip') {
            wasmModule.applyHorizontalFlip(wasmBuffer, canvas.width, canvas.height);
        } else if (currentFilter === 'sepia') {
            wasmModule.applySepia(wasmBuffer, data.length);
        } else if (currentFilter === 'invert') {
            wasmModule.applyInvert(wasmBuffer, data.length);  // <- 추가
        }

        // 3. WASM 메모리에서 JS로 결과 복사
        data.set(wasmModule.HEAPU8.subarray(wasmBuffer, wasmBuffer + data.length));

        ctx.putImageData(imageData, 0, 0);
    }

    // ... 생략 ...
}
```

> **중요**: 이 부분을 빠뜨리면 버튼은 눌리지만 필터가 적용되지 않습니다!

---

## Step 4: 빌드 및 테스트

### 4-1. Emscripten 환경 활성화

```bash
source emsdk/emsdk_env.sh
```

### 4-2. 빌드 실행

```bash
./build.sh
```

### 4-3. 서버 실행

```bash
./serve.sh
# 또는
cd build && python3 -m http.server 8080
```

### 4-4. 브라우저에서 테스트

`http://localhost:8080` 접속 후 새 필터 버튼 클릭하여 확인

---

## 체크리스트

새 필터 추가 시 아래 항목을 모두 체크하세요:

- [ ] `src/filters/filters.cpp` - 필터 함수 작성
- [ ] `src/filters/filters.cpp` - `EMSCRIPTEN_BINDINGS`에 함수 등록
- [ ] `web/index.html` - 버튼 HTML 추가
- [ ] `web/app.js` - 버튼 DOM 참조 (`const btnXxx = ...`)
- [ ] `web/app.js` - 이벤트 리스너 등록 (`setupEventListeners`)
- [ ] `web/app.js` - `setFilter()` 함수에 버튼 활성화 케이스 추가
- [ ] `web/app.js` - `processFrame()` 함수에 WASM 호출 추가
- [ ] `./build.sh` 실행
- [ ] 브라우저 테스트

---

## 필터 함수 예제

### 밝기 조절 (Brightness)

```cpp
void applyBrightness(uintptr_t dataPtr, int length, int amount) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);

    for (int i = 0; i < length; i += 4) {
        int r = data[i] + amount;
        int g = data[i + 1] + amount;
        int b = data[i + 2] + amount;

        data[i] = static_cast<uint8_t>(r < 0 ? 0 : (r > 255 ? 255 : r));
        data[i + 1] = static_cast<uint8_t>(g < 0 ? 0 : (g > 255 ? 255 : g));
        data[i + 2] = static_cast<uint8_t>(b < 0 ? 0 : (b > 255 ? 255 : b));
    }
}
```

### 블러 (단순 Box Blur)

```cpp
void applyBlur(uintptr_t dataPtr, int width, int height) {
    uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);
    int length = width * height * 4;

    // 임시 버퍼 필요
    uint8_t* temp = new uint8_t[length];
    memcpy(temp, data, length);

    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int c = 0; c < 3; c++) {  // RGB만
                int sum = 0;
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int idx = ((y + dy) * width + (x + dx)) * 4 + c;
                        sum += temp[idx];
                    }
                }
                data[(y * width + x) * 4 + c] = sum / 9;
            }
        }
    }

    delete[] temp;
}
```

---

## 트러블슈팅

### 버튼은 눌리는데 필터가 적용 안 됨
- `processFrame()` 함수에서 WASM 호출 추가했는지 확인
- `EMSCRIPTEN_BINDINGS`에 함수 등록했는지 확인
- `./build.sh` 재실행

### 콘솔에 "함수를 찾을 수 없습니다" 에러
- C++ 함수명과 바인딩명 일치 확인
- `./build.sh` 재실행

### 버튼이 화면에 안 보임
- `index.html`에 버튼 추가했는지 확인
- `./build.sh` 재실행 (웹 파일 복사)

### 버튼 클릭해도 반응 없음
- `app.js`에 이벤트 리스너 등록했는지 확인
- 버튼 ID가 일치하는지 확인
