/**
 * WebCam Filter WASM - 메인 애플리케이션
 */

// DOM 요소
const video = document.getElementById('webcam');
const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');
const statusDiv = document.getElementById('status');

// 버튼
const btnNone = document.getElementById('btnNone');
const btnGrayscale = document.getElementById('btnGrayscale');
const btnFlip = document.getElementById('btnFlip');

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
 * 프레임 처리 (메인 루프)
 */
function processFrame() {
    const startTime = performance.now();

    // 비디오를 캔버스에 그리기
    ctx.drawImage(video, 0, 0, canvas.width, canvas.height);

    // 필터 적용
    if (currentFilter !== 'none' && wasmModule) {
        const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);

        try {
            if (currentFilter === 'grayscale') {
                wasmModule.applyGrayscale(imageData);
            } else if (currentFilter === 'flip') {
                wasmModule.applyHorizontalFlip(imageData);
            }

            ctx.putImageData(imageData, 0, 0);
        } catch (error) {
            console.error('필터 적용 실패:', error);
        }
    }

    // 성능 측정
    const processingTime = performance.now() - startTime;
    processingTimeEl.textContent = `${processingTime.toFixed(2)} ms`;

    // FPS 계산 (1초마다 업데이트)
    frameCount++;
    const now = Date.now();
    if (now - fpsUpdateTime >= 1000) {
        const fps = frameCount / ((now - fpsUpdateTime) / 1000);
        fpsEl.textContent = `${fps.toFixed(1)}`;
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

    // 모든 버튼에서 active 클래스 제거
    document.querySelectorAll('.filter-btn').forEach(btn => {
        btn.classList.remove('active');
    });

    // 선택된 버튼에 active 클래스 추가
    if (filter === 'none') {
        btnNone.classList.add('active');
    } else if (filter === 'grayscale') {
        btnGrayscale.classList.add('active');
    } else if (filter === 'flip') {
        btnFlip.classList.add('active');
    }
}

/**
 * 이벤트 리스너 등록
 */
function setupEventListeners() {
    btnNone.addEventListener('click', () => setFilter('none'));
    btnGrayscale.addEventListener('click', () => setFilter('grayscale'));
    btnFlip.addEventListener('click', () => setFilter('flip'));
}

/**
 * 앱 초기화
 */
async function init() {
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
    if (animationId) {
        cancelAnimationFrame(animationId);
    }
    if (video.srcObject) {
        video.srcObject.getTracks().forEach(track => track.stop());
    }
});
