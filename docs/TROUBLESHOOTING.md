# 문제 해결 가이드

> 자주 발생하는 문제와 해결 방법

## 목차
- [설치 및 환경 설정](#설치-및-환경-설정)
- [빌드 오류](#빌드-오류)
- [실행 시 오류](#실행-시-오류)
- [성능 문제](#성능-문제)
- [웹캠 관련 문제](#웹캠-관련-문제)
- [메모리 누수](#메모리-누수)
- [디버깅 방법](#디버깅-방법)

---

## 설치 및 환경 설정

### ❌ `emcc: command not found`

**증상**:
```bash
$ emcc --version
bash: emcc: command not found
```

**원인**: Emscripten 환경 변수가 설정되지 않음

**해결 방법**:

#### 임시 해결 (현재 터미널만):
```bash
# macOS/Linux
source ~/emsdk/emsdk_env.sh

# Windows
.\emsdk_env.bat
```

#### 영구 해결:

**macOS/Linux**:
```bash
# 1. 홈 디렉토리 확인
echo $HOME

# 2. .zshrc 또는 .bashrc 편집
nano ~/.zshrc  # macOS (zsh)
# 또는
nano ~/.bashrc # Linux (bash)

# 3. 마지막 줄에 추가
source "$HOME/emsdk/emsdk_env.sh"

# 4. 저장 후 재로드
source ~/.zshrc  # macOS
source ~/.bashrc # Linux

# 5. 확인
emcc --version
```

**Windows**:
```powershell
# 시스템 환경 변수 추가
1. "환경 변수 편집" 검색
2. "Path" 선택 → 편집
3. "새로 만들기" 클릭
4. "C:\경로\to\emsdk" 추가
5. "C:\경로\to\emsdk\upstream\emscripten" 추가
6. 확인 후 터미널 재시작
```

---

### ❌ Python 버전 오류

**증상**:
```
ERROR: Python 2 is no longer supported.
Please use Python 3.8 or later.
```

**해결 방법**:

```bash
# 1. Python 3 설치 확인
python3 --version

# 2. 기본 python 명령어를 python3로 변경
# macOS/Linux
alias python=python3
alias pip=pip3

# 영구 설정
echo 'alias python=python3' >> ~/.zshrc
echo 'alias pip=pip3' >> ~/.zshrc

# 3. Emscripten 재설치
cd ~/emsdk
./emsdk install latest
./emsdk activate latest
```

---

## 빌드 오류

### ❌ `error: undefined reference to ...`

**증상**:
```
error: undefined reference to 'applyGrayscaleFilterRaw'
```

**원인**: 함수가 바인딩되지 않았거나 함수명 오타

**해결 방법**:

1. **함수 바인딩 확인** (`main.cpp`):
```cpp
EMSCRIPTEN_BINDINGS(webcam_filter) {
    function("applyGrayscaleFilterRaw", &applyGrayscaleFilterRaw);
    // ↑ 함수명 정확히 일치하는지 확인
}
```

2. **함수 구현 확인**:
```cpp
// extern "C" 또는 바인딩 필요
void applyGrayscaleFilterRaw(uintptr_t ptr, int length) {
    // 구현...
}
```

3. **재빌드**:
```bash
rm -rf build/*
./build.sh
```

---

### ❌ `error: use of undeclared identifier 'std::...'`

**증상**:
```cpp
error: use of undeclared identifier 'std::min'
error: use of undeclared identifier 'std::clamp'
```

**원인**: 필요한 헤더가 포함되지 않음

**해결 방법**:

```cpp
// main.cpp 상단에 추가
#include <algorithm>  // std::min, std::max, std::clamp
#include <cmath>      // std::abs, std::sqrt
#include <vector>     // std::vector
#include <cstring>    // std::memcpy
```

---

### ❌ 빌드 스크립트 권한 오류

**증상**:
```bash
$ ./build.sh
bash: ./build.sh: Permission denied
```

**해결 방법**:

```bash
# 실행 권한 부여
chmod +x build.sh
chmod +x serve.sh
chmod +x dev.sh

# 다시 실행
./build.sh
```

---

## 실행 시 오류

### ❌ WASM 파일 로딩 실패

**증상**:
```
CompileError: WebAssembly.instantiate():
Wasm code generation disallowed by embedder
```

**원인**: `file://` 프로토콜에서 WASM 로딩 불가

**해결 방법**:

반드시 로컬 서버 사용:

```bash
# 방법 1: Python
cd web
python3 -m http.server 8000

# 방법 2: Node.js
npx http-server web -p 8000

# 방법 3: serve.sh 스크립트
./serve.sh
```

브라우저에서: `http://localhost:8000`

❌ **잘못된 방법**: `file:///path/to/index.html`

---

### ❌ `Module._functionName is not a function`

**증상**:
```javascript
TypeError: Module._applyGrayscaleFilterRaw is not a function
```

**원인**:
1. WASM이 아직 로드되지 않음
2. 함수명 오타
3. 바인딩 누락

**해결 방법**:

#### 1. WASM 로드 대기:
```javascript
// ❌ 잘못된 코드
Module._applyGrayscaleFilterRaw(ptr, size);  // WASM 로드 전 호출

// ✅ 올바른 코드
Module.onRuntimeInitialized = function() {
    console.log('WASM ready');
    // 이제 안전하게 호출 가능
    initApp();
};
```

#### 2. 함수명 확인:
```javascript
// main.cpp의 바인딩명과 정확히 일치해야 함
EMSCRIPTEN_BINDINGS(webcam_filter) {
    function("applyGrayscaleFilterRaw", &applyGrayscaleFilterRaw);
    //        ↑ 이 이름으로 호출
}

// JavaScript
Module._applyGrayscaleFilterRaw(ptr, size);
//      ↑ 언더스코어 prefix 필수!
```

#### 3. 빌드 확인:
```bash
# 최신 빌드인지 확인
ls -lh web/filter.wasm web/filter.js

# 브라우저 캐시 삭제 후 새로고침
# Chrome: Cmd+Shift+R (Mac) / Ctrl+Shift+R (Windows)
```

---

### ❌ 검은 화면만 표시됨

**증상**: 웹캠 화면이 안 나오고 검은 화면만 표시

**해결 방법**:

#### 1. 브라우저 콘솔 확인 (F12):
```javascript
// 에러 메시지 확인
// 예: NotAllowedError, NotFoundError 등
```

#### 2. 웹캠 권한 확인:
- Chrome: 주소창 왼쪽 자물쇠 아이콘 → 카메라 → 허용
- Firefox: 주소창 왼쪽 카메라 아이콘 → 권한 확인
- Safari: 환경설정 → 웹사이트 → 카메라 → 허용

#### 3. 다른 앱에서 웹캠 사용 중인지 확인:
```bash
# macOS: 카메라 사용 중인 앱 확인
lsof | grep "Camera"

# 또는 시스템 환경설정 → 보안 및 개인정보 보호 → 카메라
```

#### 4. 웹캠 장치 테스트:
```javascript
// 브라우저 콘솔에서 실행
navigator.mediaDevices.enumerateDevices()
    .then(devices => {
        const cameras = devices.filter(d => d.kind === 'videoinput');
        console.log('사용 가능한 카메라:', cameras);
    });
```

---

### ❌ 필터가 적용되지 않음

**증상**: 버튼 클릭해도 필터 효과가 없음

**디버깅 단계**:

#### 1. 콘솔 로그 추가:
```javascript
function applyGrayscaleFilter(imageData) {
    console.log('필터 호출됨');
    console.log('ImageData:', imageData.width, imageData.height);

    const bytes = imageData.data;
    const ptr = Module._malloc(bytes.length);

    console.log('메모리 할당:', ptr);

    try {
        Module.HEAPU8.set(bytes, ptr);
        console.log('데이터 복사 완료');

        Module._applyGrayscaleFilterRaw(ptr, bytes.length);
        console.log('C++ 함수 호출 완료');

        bytes.set(Module.HEAPU8.subarray(ptr, ptr + bytes.length));
        console.log('결과 복사 완료');
    } finally {
        Module._free(ptr);
    }
}
```

#### 2. 픽셀 값 확인:
```javascript
// 필터 적용 전후 비교
console.log('Before:', bytes[0], bytes[1], bytes[2]);
// C++ 함수 호출
console.log('After:', bytes[0], bytes[1], bytes[2]);
```

#### 3. 함수 호출 확인:
```javascript
// 버튼 이벤트가 제대로 연결되었는지 확인
document.getElementById('grayscaleBtn')?.addEventListener('click', () => {
    console.log('흑백 버튼 클릭됨');  // 이 로그가 안 나오면 이벤트 문제
    currentFilter = 'grayscale';
});
```

---

## 성능 문제

### ❌ 낮은 FPS (프레임 드롭)

**증상**: 영상이 버벅이고 FPS가 30 미만

**원인 파악**:

```javascript
function processFrame() {
    const startTime = performance.now();

    // 1. 웹캠 → Canvas
    const t1 = performance.now();
    ctx.drawImage(video, 0, 0, width, height);
    console.log('drawImage:', (performance.now() - t1).toFixed(2), 'ms');

    // 2. getImageData
    const t2 = performance.now();
    const imageData = ctx.getImageData(0, 0, width, height);
    console.log('getImageData:', (performance.now() - t2).toFixed(2), 'ms');

    // 3. 필터 적용
    const t3 = performance.now();
    applyGrayscaleFilter(imageData);
    console.log('filter:', (performance.now() - t3).toFixed(2), 'ms');

    // 4. putImageData
    const t4 = performance.now();
    ctx.putImageData(imageData, 0, 0);
    console.log('putImageData:', (performance.now() - t4).toFixed(2), 'ms');

    console.log('Total:', (performance.now() - startTime).toFixed(2), 'ms');

    requestAnimationFrame(processFrame);
}
```

**해결 방법**:

#### 1. 해상도 낮추기:
```javascript
// ❌ 고해상도 (느림)
const stream = await navigator.mediaDevices.getUserMedia({
    video: { width: 1920, height: 1080 }
});

// ✅ 적절한 해상도 (빠름)
const stream = await navigator.mediaDevices.getUserMedia({
    video: { width: 1280, height: 720 }
});
```

#### 2. Canvas 최적화:
```javascript
// willReadFrequently 옵션 추가
const ctx = canvas.getContext('2d', {
    willReadFrequently: true,
    alpha: false  // Alpha 채널 불필요 시
});
```

#### 3. 메모리 재사용:
```javascript
// 매 프레임마다 할당/해제 하지 말고 재사용
let cachedPtr = 0;

function init() {
    const numBytes = canvas.width * canvas.height * 4;
    cachedPtr = Module._malloc(numBytes);
}

function processFrame() {
    // 재사용
    Module.HEAPU8.set(imageData.data, cachedPtr);
    Module._applyGrayscaleFilterRaw(cachedPtr, numBytes);
    imageData.data.set(Module.HEAPU8.subarray(cachedPtr, cachedPtr + numBytes));
}

function cleanup() {
    Module._free(cachedPtr);
}
```

#### 4. 프레임 건너뛰기:
```javascript
let frameCount = 0;

function processFrame() {
    frameCount++;

    // 2 프레임마다 1번만 필터 적용
    if (frameCount % 2 === 0) {
        applyFilter(imageData);
    }

    requestAnimationFrame(processFrame);
}
```

---

### ❌ 메모리 사용량 증가

**증상**: 시간이 지날수록 메모리 사용량이 계속 증가

**원인**: 메모리 누수

**확인 방법**:

```javascript
// Chrome DevTools → Memory 탭
// "Heap snapshot" 촬영
// 시간 경과 후 다시 촬영하여 비교
```

**해결 방법**:

#### 1. 메모리 해제 확인:
```javascript
function applyFilter(imageData) {
    const ptr = Module._malloc(imageData.data.length);

    try {
        // 처리...
    } finally {
        Module._free(ptr);  // ⭐ 반드시 해제!
    }
}
```

#### 2. 이벤트 리스너 정리:
```javascript
// ❌ 메모리 누수
function start() {
    video.addEventListener('loadeddata', () => {
        processFrame();
    });
}

// ✅ 정리
let loadedHandler = null;

function start() {
    loadedHandler = () => processFrame();
    video.addEventListener('loadeddata', loadedHandler);
}

function stop() {
    if (loadedHandler) {
        video.removeEventListener('loadeddata', loadedHandler);
        loadedHandler = null;
    }
}
```

#### 3. requestAnimationFrame 정리:
```javascript
let rafId = null;

function startProcessing() {
    function loop() {
        processFrame();
        rafId = requestAnimationFrame(loop);
    }
    loop();
}

function stopProcessing() {
    if (rafId !== null) {
        cancelAnimationFrame(rafId);
        rafId = null;
    }
}
```

---

## 웹캠 관련 문제

### ❌ `NotAllowedError: Permission denied`

**증상**: 웹캠 권한 거부 오류

**해결 방법**:

#### 1. 브라우저 권한 재설정:
- Chrome: chrome://settings/content/camera
- Firefox: about:preferences#privacy → 권한 → 카메라
- Safari: 환경설정 → 웹사이트 → 카메라

#### 2. HTTPS 확인:
```javascript
// ❌ HTTP에서는 권한 요청 제한됨
http://example.com

// ✅ HTTPS 또는 localhost 사용
https://example.com
http://localhost:8000  // localhost는 예외
```

#### 3. 권한 요청 코드:
```javascript
async function requestCamera() {
    try {
        const stream = await navigator.mediaDevices.getUserMedia({
            video: true
        });
        console.log('카메라 권한 허용됨');
        return stream;

    } catch (error) {
        if (error.name === 'NotAllowedError') {
            alert('카메라 권한을 허용해주세요.');
        } else if (error.name === 'NotFoundError') {
            alert('카메라를 찾을 수 없습니다.');
        } else {
            console.error('카메라 오류:', error);
        }
        throw error;
    }
}
```

---

### ❌ `NotFoundError: Requested device not found`

**증상**: 웹캠을 찾을 수 없음

**해결 방법**:

#### 1. 웹캠 연결 확인:
```bash
# macOS
system_profiler SPCameraDataType

# Linux
ls -l /dev/video*

# Windows: 장치 관리자에서 카메라 확인
```

#### 2. 사용 가능한 카메라 나열:
```javascript
async function listCameras() {
    const devices = await navigator.mediaDevices.enumerateDevices();
    const cameras = devices.filter(device => device.kind === 'videoinput');

    console.log('사용 가능한 카메라:');
    cameras.forEach((camera, index) => {
        console.log(`${index}: ${camera.label || 'Camera ' + index}`);
    });

    return cameras;
}
```

#### 3. 특정 카메라 선택:
```javascript
const cameras = await listCameras();
const selectedCamera = cameras[0];  // 첫 번째 카메라

const stream = await navigator.mediaDevices.getUserMedia({
    video: { deviceId: selectedCamera.deviceId }
});
```

---

## 메모리 누수

### 디버깅 도구

```javascript
// WASM 메모리 사용량 모니터링
function monitorMemory() {
    setInterval(() => {
        const usedMB = Module.HEAPU8.length / (1024 * 1024);
        console.log(`WASM Memory: ${usedMB.toFixed(2)} MB`);

        // JavaScript 메모리
        if (performance.memory) {
            const jsUsedMB = performance.memory.usedJSHeapSize / (1024 * 1024);
            console.log(`JS Memory: ${jsUsedMB.toFixed(2)} MB`);
        }
    }, 5000);  // 5초마다
}

monitorMemory();
```

---

## 디버깅 방법

### Chrome DevTools 활용

#### 1. Console 탭:
```javascript
// 로그 레벨 설정
console.log('정보');
console.warn('경고');
console.error('에러');

// 그룹화
console.group('필터 처리');
console.log('Step 1');
console.log('Step 2');
console.groupEnd();

// 시간 측정
console.time('filter');
applyGrayscaleFilter(imageData);
console.timeEnd('filter');  // filter: 3.2ms
```

#### 2. Performance 탭:
1. Record 버튼 클릭
2. 필터 적용
3. Stop 버튼 클릭
4. 타임라인에서 느린 함수 확인

#### 3. Memory 탭:
1. "Heap snapshot" 촬영
2. 10초 대기
3. 다시 촬영
4. Comparison 뷰에서 증가한 객체 확인

---

### C++ 디버그 출력

```cpp
#include <emscripten/console.h>

void applyGrayscaleFilterRaw(uintptr_t ptr, int length) {
    emscripten_console_log("=== Grayscale Filter ===");
    emscripten_console_logf("Length: %d bytes", length);

    uint8_t* data = reinterpret_cast<uint8_t*>(ptr);

    // 첫 픽셀 출력
    emscripten_console_logf(
        "First pixel - R:%d G:%d B:%d A:%d",
        data[0], data[1], data[2], data[3]
    );

    // 처리...

    emscripten_console_log("Filter applied successfully");
}
```

---

## 빠른 체크리스트

문제가 발생하면 다음을 순서대로 확인하세요:

### 빌드 문제
- [ ] Emscripten 환경 변수 설정됨?
- [ ] CMake 설치됨?
- [ ] build.sh 실행 권한 있음?
- [ ] 에러 메시지 확인함?

### 실행 문제
- [ ] 로컬 서버 실행 중? (file:// 아님)
- [ ] 브라우저 콘솔에 에러 없음?
- [ ] WASM 파일 정상 로드됨?
- [ ] Module.onRuntimeInitialized 대기함?

### 웹캠 문제
- [ ] 브라우저 카메라 권한 허용됨?
- [ ] 웹캠 연결 확인됨?
- [ ] 다른 앱에서 사용 중 아님?
- [ ] HTTPS 또는 localhost 사용?

### 성능 문제
- [ ] FPS 측정함?
- [ ] 해상도 적절함? (720p 권장)
- [ ] 메모리 재사용함?
- [ ] 프로파일링 수행함?

---

## 추가 도움

### 커뮤니티
- [Emscripten GitHub Issues](https://github.com/emscripten-core/emscripten/issues)
- [Stack Overflow - WebAssembly](https://stackoverflow.com/questions/tagged/webassembly)
- [WebAssembly Discord](https://discord.gg/webassembly)

### 공식 문서
- [Emscripten Troubleshooting](https://emscripten.org/docs/getting_started/FAQ.html)
- [MDN WebAssembly](https://developer.mozilla.org/en-US/docs/WebAssembly)

---

## 문제가 해결되지 않으면?

1. **에러 메시지 복사**
2. **재현 단계 작성**
3. **시스템 정보 수집**:
   ```bash
   emcc --version
   cmake --version
   node --version  # 사용 시
   python3 --version
   ```
4. **GitHub Issues에 질문 작성**

문제 해결에 도움이 되었기를 바랍니다! 🎉
