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
const btnGrayscale = document.getElementById('btnGrayscale');
const btnFlip = document.getElementById('btnFlip');
const btnSepia = document.getElementById('btnSepia');
const btnPixelate = document.getElementById('btnPixelate');
const btnGlitch = document.getElementById('btnGlitch');
const btnThermal = document.getElementById('btnThermal');
const btnNightVision = document.getElementById('btnNightVision');
const btnToon = document.getElementById('btnToon');
const btnMirror = document.getElementById('btnMirror');
const btnOldTV = document.getElementById('btnOldTV');
const btnVHS = document.getElementById('btnVHS');

// 크로마키 버튼 동적 생성
const btnChroma = document.createElement('button');
btnChroma.id = 'btnChroma';
btnChroma.className = 'filter-btn';
btnChroma.textContent = '크로마키';
btnChroma.setAttribute('aria-pressed', 'false');
document.querySelector('.filter-buttons').appendChild(btnChroma);

// 크로마키 UI 요소 생성
const chromaControls = document.createElement('section');
chromaControls.className = 'chroma-controls';
chromaControls.innerHTML = `
    <div class="chroma-row">
        <label for="chromaColor">배경 색상</label>
        <input type="color" id="chromaColor" value="#00ff00">
    </div>
    <div class="chroma-row">
        <label for="chromaTolerance">허용 범위</label>
        <input type="range" id="chromaTolerance" min="10" max="150" value="60">
        <span id="chromaToleranceValue">60</span>
    </div>
    <div class="chroma-row">
        <label for="chromaFile">배경 이미지</label>
        <input type="file" id="chromaFile" accept="image/*">
    </div>
`;
document.querySelector('.controls').appendChild(chromaControls);

// 성능 측정
const processingTimeEl = document.getElementById('processingTime');
const fpsEl = document.getElementById('fps');

// 상태 관리
let currentFilter = 'none';
let wasmModule = null;
let animationId = null;
let lastFrameTime = Date.now();
let frameCount = 0;
let fpsUpdateTime = Date.now();

// WASM 메모리 버퍼 (재사용으로 할당 오버헤드 최소화)
let wasmBuffer = null;
let wasmBufferSize = 0;

// 필터별 설정
const filterSettings = {
    pixelate: { blockSize: 8 },
    glitch: { intensity: 50, seed: 0 },
    nightvision: { seed: 0 },
    toon: { levels: 5 },
    mirror: { mode: 2 },  // 0=좌우, 1=상하, 2=4분할
    oldtv: { seed: 0 },
    vhs: { seed: 0 }
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
        if (currentFilter === 'grayscale') {
            wasmModule.applyGrayscale(wasmBuffer, data.length);
        } else if (currentFilter === 'flip') {
            wasmModule.applyHorizontalFlip(wasmBuffer, canvas.width, canvas.height);
        } else if (currentFilter === 'sepia') {
            wasmModule.applySepia(wasmBuffer, data.length);
        } else if (currentFilter === 'pixelate') {
            wasmModule.applyPixelate(wasmBuffer, canvas.width, canvas.height, filterSettings.pixelate.blockSize);
        } else if (currentFilter === 'glitch') {
            // 글리치는 매 프레임마다 다른 시드로 애니메이션 효과
            filterSettings.glitch.seed = Date.now() % 10000;
            wasmModule.applyGlitch(wasmBuffer, canvas.width, canvas.height, filterSettings.glitch.intensity, filterSettings.glitch.seed);
        } else if (currentFilter === 'thermal') {
            wasmModule.applyThermal(wasmBuffer, data.length);
        } else if (currentFilter === 'nightvision') {
            filterSettings.nightvision.seed = Date.now() % 10000;
            wasmModule.applyNightVision(wasmBuffer, canvas.width, canvas.height, filterSettings.nightvision.seed);
        } else if (currentFilter === 'toon') {
            wasmModule.applyToon(wasmBuffer, canvas.width, canvas.height, filterSettings.toon.levels);
        } else if (currentFilter === 'mirror') {
            wasmModule.applyMirror(wasmBuffer, canvas.width, canvas.height, filterSettings.mirror.mode);
        } else if (currentFilter === 'oldtv') {
            filterSettings.oldtv.seed = Date.now() % 10000;
            wasmModule.applyOldTV(wasmBuffer, canvas.width, canvas.height, filterSettings.oldtv.seed);
        } else if (currentFilter === 'vhs') {
            filterSettings.vhs.seed = Date.now() % 10000;
            wasmModule.applyVHS(wasmBuffer, canvas.width, canvas.height, filterSettings.vhs.seed);
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
        'grayscale': btnGrayscale,
        'flip': btnFlip,
        'sepia': btnSepia,
        'pixelate': btnPixelate,
        'glitch': btnGlitch,
        'thermal': btnThermal,
        'nightvision': btnNightVision,
        'toon': btnToon,
        'mirror': btnMirror,
        'oldtv': btnOldTV,
        'vhs': btnVHS,
        'chroma': btnChroma
    };

    const activeBtn = filterBtnMap[filter];
    if (activeBtn) {
        activeBtn.classList.add('active');
        activeBtn.setAttribute('aria-pressed', 'true');
    }

    // 크로마키 UI 표시/숨김
    chromaControls.style.display = (filter === 'chroma') ? 'block' : 'none';
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
    btnGrayscale.addEventListener('click', () => setFilter('grayscale'));
    btnFlip.addEventListener('click', () => setFilter('flip'));
    btnSepia.addEventListener('click', () => setFilter('sepia'));
    btnPixelate.addEventListener('click', () => setFilter('pixelate'));
    btnGlitch.addEventListener('click', () => setFilter('glitch'));
    btnThermal.addEventListener('click', () => setFilter('thermal'));
    btnNightVision.addEventListener('click', () => setFilter('nightvision'));
    btnToon.addEventListener('click', () => setFilter('toon'));
    btnMirror.addEventListener('click', () => setFilter('mirror'));
    btnOldTV.addEventListener('click', () => setFilter('oldtv'));
    btnVHS.addEventListener('click', () => setFilter('vhs'));
    btnChroma.addEventListener('click', () => setFilter('chroma'));

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

// 페이지 언로드 시 정리
window.addEventListener('beforeunload', () => {
    if (animationId) cancelAnimationFrame(animationId);
    if (wasmModule && wasmBuffer) wasmModule.freeBuffer(wasmBuffer);
    if (wasmModule && chromaBgBuffer) wasmModule.freeBuffer(chromaBgBuffer);
    if (video.srcObject) video.srcObject.getTracks().forEach(track => track.stop());
});
