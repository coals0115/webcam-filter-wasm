/**
 * WebCam Filter WASM - 메인 애플리케이션
 * 고성능 WASM 메모리 직접 접근 방식 사용
 */

// DOM 요소
const video = document.getElementById('webcam');
const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d', { willReadFrequently: true });
const statusDiv = document.getElementById('status');

// 버튼
const btnNone = document.getElementById('btnNone');
const btnSepia = document.getElementById('btnSepia');
const btnXray = document.getElementById('btnXray');
const btnMirror = document.getElementById('btnMirror');
const btnGrayscale = document.getElementById('btnGrayscale');
const btnChroma = document.getElementById('btnChroma');
const btnThermal = document.getElementById('btnThermal');

// 크로마키 UI 요소
const chromaControls = document.querySelector('.chroma-controls');
const chromaColorInput = document.getElementById('chromaColor');
const chromaToleranceInput = document.getElementById('chromaTolerance');
const chromaToleranceValue = document.getElementById('chromaToleranceValue');
const chromaFileInput = document.getElementById('chromaFile');

// 크로마키 옵션 초기 상태는 비활성화 (버튼 선택 시에만 표시)
if (chromaControls) {
    chromaControls.style.display = 'none';
}

// 성능 측정
const processingTimeEl = document.getElementById('processingTime');
const fpsEl = document.getElementById('fps');

// 상태 관리
let currentFilter = 'none';
let wasmModule = null;
let animationId = null;
let frameCount = 0;
let fpsUpdateTime = Date.now();

// WASM 메모리 버퍼 (재사용으로 할당 오버헤드 최소화)
let wasmBuffer = null;
let wasmBufferSize = 0;

// 필터별 설정
const filterSettings = {
    mirror: { mode: 2 }  // 0=좌우, 1=상하, 2=4분할
};

// 크로마키 상태
let chromaColor = { r: 0, g: 255, b: 0 };
let chromaTolerance = 60;
let chromaBgBuffer = null;
let chromaBgBufferSize = 0;
let chromaLoaded = false;

/**
 * WebAssembly 모듈 로딩
 */
async function loadWasmModule() {
    try {
        statusDiv.innerHTML = '<p>WebAssembly 모듈 로딩 중...</p>';

        // Emscripten MODULARIZE 모드: Module은 async 함수
        if (typeof Module !== 'function') {
            throw new Error('WebAssembly 모듈을 찾을 수 없습니다. filters.js가 로드되었는지 확인하세요.');
        }

        // Module 초기화 (MODULARIZE 모드에서는 함수 호출 필요)
        wasmModule = await Module();

        statusDiv.innerHTML = '<p class="success">✅ WebAssembly 모듈 로딩 완료</p>';
        statusDiv.classList.add('success');

        return true;
    } catch (error) {
        console.error('WASM 로딩 실패:', error);
        statusDiv.innerHTML = `<p class="error">❌ 오류: ${error.message}</p>`;
        statusDiv.classList.add('error');
        return false;
    }
}

/**
 * 웹캠 초기화
 */
async function initWebcam() {
    try {
        statusDiv.innerHTML = '<p>웹캠 접근 중...</p>';

        video.srcObject = await navigator.mediaDevices.getUserMedia({
            video: {
                width: {ideal: 640},
                height: {ideal: 480},
                facingMode: 'user'
            },
            audio: false
        });

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
        wasmBufferSize = bufferSize;

        statusDiv.innerHTML = '<p class="success">✅ 웹캠 연결 완료</p>';
        statusDiv.classList.add('success');

        // 프레임 처리 시작
        processFrame();

        return true;
    } catch (error) {
        console.error('웹캠 초기화 실패:', error);
        statusDiv.innerHTML = `<p class="error">❌ 웹캠 접근 실패: ${error.message}</p>`;
        statusDiv.classList.add('error');
        return false;
    }
}

/**
 * 프레임 처리 (메인 루프) - 고성능 버전
 */
function processFrame() {
    const startTime = performance.now();

    // 비디오를 캔버스에 그리기
    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);

    // 필터 적용
    if (currentFilter !== 'none' && wasmModule && wasmBuffer) {
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
        const data = imageData.data;

        // 1. JS 데이터를 WASM 메모리로 복사 (한 번만)
        wasmModule.HEAPU8.set(data, wasmBuffer);

        // 2. WASM에서 필터 처리 (순수 C++ 연산, JS 호출 없음)
        if (currentFilter === 'sepia') {
            wasmModule.applySepia(wasmBuffer, data.length);
        } else if (currentFilter === 'xray') {
            wasmModule.applyXrayFilter(wasmBuffer, data.length);
        } else if (currentFilter === 'mirror') {
            wasmModule.applyMirror(wasmBuffer, canvas.width, canvas.height, filterSettings.mirror.mode);
        } else if (currentFilter === 'grayscale') {
            wasmModule.applyGrayscale(wasmBuffer, data.length);
        } else if (currentFilter === 'chroma' && chromaLoaded && chromaBgBuffer) {
            wasmModule.applyChromaKey(
                wasmBuffer,
                chromaBgBuffer,
                canvas.width,
                canvas.height,
                chromaColor.r,
                chromaColor.g,
                chromaColor.b,
                chromaTolerance
            );
        } else if (currentFilter === 'thermal') {
            wasmModule.applyThermal(wasmBuffer, data.length);
        }

        // 3. WASM 메모리에서 JS로 결과 복사 (한 번만)
        data.set(wasmModule.HEAPU8.subarray(wasmBuffer, wasmBuffer + data.length));
        ctx.putImageData(imageData, 0, 0);
    }

    // 성능 측정
    const processingTime = performance.now() - startTime;
    processingTimeEl.textContent = `${processingTime.toFixed(2)} ms`;

    // Performance-based color coding for processing time
    processingTimeEl.className = 'stat-value';
    if (processingTime < 10) {
        processingTimeEl.classList.add('perf-excellent');
    } else if (processingTime < 20) {
        processingTimeEl.classList.add('perf-good');
    } else if (processingTime < 33) {
        processingTimeEl.classList.add('perf-warning');
    } else {
        processingTimeEl.classList.add('perf-critical');
    }

    // FPS 계산 (1초마다 업데이트)
    frameCount++;
    const now = Date.now();
    if (now - fpsUpdateTime >= 1000) {
        const fps = frameCount / ((now - fpsUpdateTime) / 1000);
        fpsEl.textContent = `${fps.toFixed(1)}`;

        // Performance-based color coding for FPS
        fpsEl.className = 'stat-value';
        if (fps >= 55) fpsEl.classList.add('perf-excellent');
        else if (fps >= 40) fpsEl.classList.add('perf-good');
        else if (fps >= 25) fpsEl.classList.add('perf-warning');
        else fpsEl.classList.add('perf-critical');

        frameCount = 0;
        fpsUpdateTime = now;
    }

    // 다음 프레임 요청
    animationId = requestAnimationFrame(processFrame);
}

/**
 * 필터 변경
 */
function setFilter(filter) {
    currentFilter = filter;

    // 모든 버튼에서 active 클래스 제거 및 aria-pressed 업데이트
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.classList.remove('active');
        btn.setAttribute('aria-pressed', 'false');
    });

    // 선택된 버튼에 active 클래스 추가 및 aria-pressed 업데이트
    const filterBtnMap = {
        'none': btnNone,
        'sepia': btnSepia,
        'xray': btnXray,
        'mirror': btnMirror,
        'grayscale': btnGrayscale,
        'chroma': btnChroma,
        'thermal': btnThermal
    };

    const activeBtn = filterBtnMap[filter];
    if (activeBtn) {
        activeBtn.classList.add('active');
        activeBtn.setAttribute('aria-pressed', 'true');
    }

    // 크로마키 필터 선택 여부에 따라 옵션 표시/숨김 처리
    if (chromaControls) {
        if (filter === 'chroma') {
            chromaControls.style.display = 'flex';
        } else {
            chromaControls.style.display = 'none';
        }
    }
}

/**
 * 크로마키 설정
 */
function setupChromaEvents() {
    const colorInput = document.getElementById('chromaColor');
    const toleranceInput = document.getElementById('chromaTolerance');
    const toleranceValue = document.getElementById('chromaToleranceValue');
    const fileInput = document.getElementById('chromaFile');

    colorInput.addEventListener('input', () => {
        const hex = colorInput.value.replace('#', '');
        chromaColor = {
            r: parseInt(hex.substring(0, 2), 16),
            g: parseInt(hex.substring(2, 4), 16),
            b: parseInt(hex.substring(4, 6), 16)
        };
    });

    toleranceInput.addEventListener('input', () => {
        chromaTolerance = Number(toleranceInput.value);
        toleranceValue.textContent = String(chromaTolerance);
    });

    fileInput.addEventListener('change', (e) => {
        const file = e.target.files && e.target.files[0];
        if (!file) return;

        const img = new Image();
        img.onload = () => {
            const offCanvas = document.createElement('canvas');
            offCanvas.width = canvas.width;
            offCanvas.height = canvas.height;
            const offCtx = offCanvas.getContext('2d');
            offCtx.drawImage(img, 0, 0, offCanvas.width, offCanvas.height);

            const bgData = offCtx.getImageData(0, 0, offCanvas.width, offCanvas.height).data;
            const size = bgData.length;

            if (!chromaBgBuffer || chromaBgBufferSize !== size) {
                if (chromaBgBuffer) wasmModule.freeBuffer(chromaBgBuffer);
                chromaBgBuffer = wasmModule.allocateBuffer(size);
                chromaBgBufferSize = size;
            }

            wasmModule.HEAPU8.set(bgData, chromaBgBuffer);
            chromaLoaded = true;

            statusDiv.innerHTML = '<p class="success">🎨 크로마키 배경 이미지 적용 완료</p>';
            statusDiv.classList.add('success');
        };

        img.src = URL.createObjectURL(file);
    });
}

/**
 * 이벤트 리스너 등록
 */
function setupEventListeners() {
    btnNone.addEventListener('click', () => setFilter('none'));
    btnSepia.addEventListener('click', () => setFilter('sepia'));
    btnXray.addEventListener('click', () => setFilter('xray'));
    btnMirror.addEventListener('click', () => setFilter('mirror'));
    btnGrayscale.addEventListener('click', () => setFilter('grayscale'));
    btnChroma.addEventListener('click', () => setFilter('chroma'));
    btnThermal.addEventListener('click', () => setFilter('thermal'));

    setupChromaEvents();
}

/**
 * 앱 초기화
 */
async function init() {
    // Startup animation - fade in container
    const container = document.querySelector('.container');
    container.style.opacity = '0';
    container.style.transform = 'translateY(20px)';

    setTimeout(() => {
        container.style.transition = 'opacity 0.8s ease, transform 0.8s ease';
        container.style.opacity = '1';
        container.style.transform = 'translateY(0)';
    }, 100);

    // 크로마키 UI 초기 숨김
    chromaControls.style.display = 'none';

    setupEventListeners();

    // WebAssembly 모듈 로딩
    const wasmLoaded = await loadWasmModule();
    if (!wasmLoaded) {
        return;
    }

    // 웹캠 초기화
    const webcamReady = await initWebcam();
    if (!webcamReady) {
        return;
    }

    console.log('✅ 애플리케이션 초기화 완료');
}

// DOM 로딩 완료 후 초기화
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}

// ===== JS 필터 (벤치마크용) =====

// 0. Grayscale - C++과 동일: ITU-R BT.601 표준
function applyGrayscaleJS(data) {
    for (let i = 0; i < data.length; i += 4) {
        const r = data[i], g = data[i + 1], b = data[i + 2];
        // gray = 0.299*R + 0.587*G + 0.114*B (float 버전)
        const gray = 0.299 * r + 0.587 * g + 0.114 * b;
        data[i] = data[i + 1] = data[i + 2] = gray;
    }
}

// 1. Sepia
function applySepiaJS(data) {
    for (let i = 0; i < data.length; i += 4) {
        const r = data[i], g = data[i + 1], b = data[i + 2];
        data[i] = Math.min(255, 0.393 * r + 0.769 * g + 0.189 * b);
        data[i + 1] = Math.min(255, 0.349 * r + 0.686 * g + 0.168 * b);
        data[i + 2] = Math.min(255, 0.272 * r + 0.534 * g + 0.131 * b);
    }
}

// 2. X-Ray (C++과 동일한 알고리즘: RGB 각각 반전 + 대비 증강)
function applyXrayJS(data) {
    const contrast_factor = 80;
    for (let i = 0; i < data.length; i += 4) {
        // 1. 명암 반전
        const r = 255 - data[i];
        const g = 255 - data[i + 1];
        const b = 255 - data[i + 2];
        // 2. 대비 증강 (C++과 동일: 128 + ((val - 128) * 80) / 128)
        let newR = 128 + ((r - 128) * contrast_factor / 128) | 0;
        let newG = 128 + ((g - 128) * contrast_factor / 128) | 0;
        let newB = 128 + ((b - 128) * contrast_factor / 128) | 0;
        // 3. 클램핑
        data[i] = Math.max(0, Math.min(255, newR));
        data[i + 1] = Math.max(0, Math.min(255, newG));
        data[i + 2] = Math.max(0, Math.min(255, newB));
    }
}

// 3. Mirror (4분할) - C++과 동일: 1루프에서 3영역 동시 복사
function applyMirrorJS(data, width, height) {
    const halfW = Math.floor(width / 2);
    const halfH = Math.floor(height / 2);

    for (let y = 0; y < halfH; y++) {
        for (let x = 0; x < halfW; x++) {
            const srcIdx = (y * width + x) * 4;
            // 우상단 (좌우 반전)
            const idx1 = (y * width + (width - 1 - x)) * 4;
            // 좌하단 (상하 반전)
            const idx2 = ((height - 1 - y) * width + x) * 4;
            // 우하단 (좌우+상하 반전)
            const idx3 = ((height - 1 - y) * width + (width - 1 - x)) * 4;

            for (let c = 0; c < 4; c++) {
                data[idx1 + c] = data[srcIdx + c];
                data[idx2 + c] = data[srcIdx + c];
                data[idx3 + c] = data[srcIdx + c];
            }
        }
    }
}

// 4. Thermal - C++과 동일: 정수 연산 (밝기 계산 + 색상 매핑)
function applyThermalJS(data) {
    for (let i = 0; i < data.length; i += 4) {
        // 밝기 계산 - C++과 동일: (r*54 + g*183 + b*19) >> 8
        const brightness = (data[i] * 54 + data[i + 1] * 183 + data[i + 2] * 19) >> 8;
        let r, g, b;

        // 색상 매핑 - C++과 동일: (t * 255) / range
        if (brightness < 43) {
            r = 0;
            g = 0;
            b = ((brightness * 255) / 43) | 0;
        } else if (brightness < 85) {
            const t = brightness - 43;
            r = 0;
            g = ((t * 255) / 42) | 0;
            b = 255;
        } else if (brightness < 128) {
            const t = brightness - 85;
            r = 0;
            g = 255;
            b = (255 - ((t * 255) / 43)) | 0;
        } else if (brightness < 170) {
            const t = brightness - 128;
            r = ((t * 255) / 42) | 0;
            g = 255;
            b = 0;
        } else if (brightness < 213) {
            const t = brightness - 170;
            r = 255;
            g = (255 - ((t * 255) / 43)) | 0;
            b = 0;
        } else {
            const t = brightness - 213;
            const val = ((t * 255) / 42) | 0;
            r = 255;
            g = val;
            b = val;
        }
        data[i] = r;
        data[i + 1] = g;
        data[i + 2] = b;
    }
}

// 6. Chroma Key
function applyChromaKeyJS(frameData, bgData, keyR, keyG, keyB, tolerance) {
    const tolSq = tolerance * tolerance;
    for (let i = 0; i < frameData.length; i += 4) {
        const dr = frameData[i] - keyR;
        const dg = frameData[i + 1] - keyG;
        const db = frameData[i + 2] - keyB;
        if (dr * dr + dg * dg + db * db <= tolSq) {
            frameData[i] = bgData[i];
            frameData[i + 1] = bgData[i + 1];
            frameData[i + 2] = bgData[i + 2];
            frameData[i + 3] = bgData[i + 3];
        }
    }
}

// ===== 벤치마크 함수 =====
async function runBenchmark(iterations = 50) {
    console.log(`🚀 벤치마크 시작 (${iterations}회 반복, ${canvas.width}x${canvas.height})`);

    const results = [];
    const bufferSize = canvas.width * canvas.height * 4;

    const filters = [
        {
            name: 'Grayscale',
            wasmFn: () => wasmModule.applyGrayscale(wasmBuffer, bufferSize),
            jsFn: (d) => applyGrayscaleJS(d)
        },
        {
            name: 'Sepia',
            wasmFn: () => wasmModule.applySepia(wasmBuffer, bufferSize),
            jsFn: (d) => applySepiaJS(d)
        },
        {
            name: 'X-Ray',
            wasmFn: () => wasmModule.applyXrayFilter(wasmBuffer, bufferSize),
            jsFn: (d) => applyXrayJS(d)
        },
        {
            name: 'Mirror',
            wasmFn: () => wasmModule.applyMirror(wasmBuffer, canvas.width, canvas.height, 2),
            jsFn: (d) => applyMirrorJS(d, canvas.width, canvas.height)
        },
        {
            name: 'Thermal',
            wasmFn: () => wasmModule.applyThermal(wasmBuffer, bufferSize),
            jsFn: (d) => applyThermalJS(d)
        }
    ];

    for (const filter of filters) {
        // WASM 측정
        const wasmTimes = [];
        for (let i = 0; i < iterations; i++) {
            const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
            wasmModule.HEAPU8.set(imageData.data, wasmBuffer);
            const start = performance.now();
            filter.wasmFn();
            wasmTimes.push(performance.now() - start);
        }

        // JS 측정
        const jsTimes = [];
        for (let i = 0; i < iterations; i++) {
            const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
            const start = performance.now();
            filter.jsFn(imageData.data);
            jsTimes.push(performance.now() - start);
        }

        const wasmAvg = wasmTimes.reduce((a, b) => a + b) / iterations;
        const jsAvg = jsTimes.reduce((a, b) => a + b) / iterations;

        results.push({
            '필터': filter.name,
            'C++ (ms)': wasmAvg.toFixed(2),
            'JS (ms)': jsAvg.toFixed(2),
            '성능 향상': (jsAvg / wasmAvg).toFixed(1) + 'x'
        });
    }

    console.table(results);
    console.log('✅ 벤치마크 완료');
    return results;
}

// 전역에 노출 (콘솔에서 실행 가능)
window.runBenchmark = runBenchmark;

// 페이지 언로드 시 정리
window.addEventListener('beforeunload', () => {
    if (animationId) cancelAnimationFrame(animationId);
    if (wasmModule && wasmBuffer) wasmModule.freeBuffer(wasmBuffer);
    if (wasmModule && chromaBgBuffer) wasmModule.freeBuffer(chromaBgBuffer);
    if (video.srcObject) video.srcObject.getTracks().forEach(track => track.stop());
});
