# 🎥 WebCam Filter WASM

> 실시간 웹캠 필터 - WebAssembly + C++로 구현한 고성능 이미지 처리

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![WebAssembly](https://img.shields.io/badge/WebAssembly-654FF0?logo=webassembly&logoColor=white)](https://webassembly.org/)
[![C++](https://img.shields.io/badge/C++-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)

웹 브라우저에서 실시간으로 웹캠 영상에 필터를 적용하는 프로젝트입니다. C++로 작성된 이미지 처리 알고리즘을 WebAssembly로 컴파일하여 네이티브급 성능을 제공합니다.

---

## ✨ 주요 기능

### 이미지 필터
- **흑백 (Grayscale)**: ITU-R BT.709 표준 기반 밝기 변환
- **블러 (Blur)**: Box Blur 알고리즘 (커스터마이징 가능한 반경)
- **엣지 검출 (Edge Detection)**: Sobel 연산자 기반 경계선 검출
- **실시간 프리뷰**: 30 FPS 이상 성능 보장

### 성능 모니터링
- 프레임당 처리 시간 측정 (밀리초)
- 실시간 FPS (Frames Per Second) 표시
- WebAssembly 성능 최적화 적용

---

## 🚀 빠른 시작

### 사전 요구사항

#### macOS
- **OS**: Catalina 이상 (Intel/Apple Silicon)
- **Python 3.x**: 개발 서버용
- **Git**: 소스 코드 관리
- **Xcode Command Line Tools**: `xcode-select --install`
- **모던 웹 브라우저**: Chrome, Firefox, Safari (WebAssembly 지원)

#### Windows
- **OS**: Windows 10 이상
- **Python 3.x**: [python.org](https://www.python.org/downloads/)에서 설치
- **Git**: [git-scm.com](https://git-scm.com/download/win)에서 설치
- **Git Bash** (권장): Git 설치 시 함께 설치됨
- **모던 웹 브라우저**: Chrome, Firefox, Edge (WebAssembly 지원)

---

### ⚡ Quick Start (간단 테스트)

**처음 실행해보는 경우 (3분 소요)**

#### macOS / Linux

```bash
# 1. Emscripten 환경 활성화 (emsdk가 이미 설치되어 있어야 함)
cd emsdk && source ./emsdk_env.sh && cd ..

# 2. Hello World 컴파일
emcc main.cpp -o hello.html

# 3. 웹 서버 실행 (별도 터미널 창에서 실행)
python3 -m http.server 8080

# 4. 브라우저에서 열기
# http://localhost:8080/hello.html
```

> ⚠️ **emsdk가 없다면**:
> ```bash
> git clone https://github.com/emscripten-core/emsdk.git
> cd emsdk
> ./emsdk install latest
> ./emsdk activate latest
> source ./emsdk_env.sh
> cd ..
> ```

#### Windows (PowerShell)

```powershell
# 1. Emscripten 환경 활성화 (emsdk가 이미 설치되어 있어야 함)
cd emsdk
.\emsdk_env.ps1
cd ..

# 2. Hello World 컴파일
emcc main.cpp -o hello.html

# 3. 웹 서버 실행 (별도 PowerShell 창에서 실행)
python -m http.server 8080

# 4. 브라우저에서 열기
# http://localhost:8080/hello.html
```

> ⚠️ **emsdk가 없다면**:
> ```powershell
> git clone https://github.com/emscripten-core/emsdk.git
> cd emsdk
> .\emsdk install latest
> .\emsdk activate latest
> .\emsdk_env.ps1
> cd ..
> ```

#### Windows (Git Bash - 권장)

```bash
# macOS/Linux와 동일한 명령어 사용 가능
cd emsdk && source ./emsdk_env.sh && cd ..
emcc main.cpp -o hello.html
python -m http.server 8080
```

**예상 결과**: 브라우저 콘솔에 "Hello and welcome to C++!" 출력

---

### 🎥 웹캠 필터 실행 (전체 프로젝트)

#### macOS / Linux

```bash
# 1. Emscripten 환경 활성화
cd emsdk && source ./emsdk_env.sh && cd ..

# 2. 프로젝트 빌드
./build.sh

# 3. 개발 서버 실행
./serve.sh

# 4. 브라우저에서 접속
# http://localhost:8080
```

**간편 실행 (자동화 스크립트)**:
```bash
./dev.sh  # 빌드 + 서버 실행 한 번에
```

#### Windows (Git Bash - 권장)

```bash
# macOS/Linux와 동일
cd emsdk && source ./emsdk_env.sh && cd ..
./build.sh
./serve.sh
```

#### Windows (PowerShell)

```powershell
# 1. Emscripten 환경 활성화
cd emsdk
.\emsdk_env.ps1
cd ..

# 2. 수동 빌드 (build.sh 대신)
emcc src/filters/grayscale.cpp -o build/filters.js -O3 --bind `
  -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 `
  -s EXPORT_NAME="Module" -s EXPORTED_RUNTIME_METHODS='["cwrap","ccall"]'

# 3. 웹 파일 복사
New-Item -ItemType Directory -Force -Path build
Copy-Item web/* build/

# 4. 개발 서버 실행
cd build
python -m http.server 8080

# 5. 브라우저에서 접속
# http://localhost:8080
```

> 💡 **Tip**: Windows에서는 **Git Bash**를 사용하는 것을 강력히 권장합니다. PowerShell보다 훨씬 간편합니다!

---

## 📁 프로젝트 구조

```
webcam-filter-wasm/
├── emsdk/                      # Emscripten SDK
├── src/                        # C++ 소스 코드
│   ├── filters/                # 필터 구현
│   │   ├── grayscale.cpp       # 흑백 필터
│   │   ├── blur.cpp            # 블러 필터
│   │   └── edge.cpp            # 엣지 검출 필터
│   └── main.cpp                # 메인 진입점 + 바인딩
├── web/                        # 웹 프론트엔드
│   ├── index.html              # 메인 HTML
│   ├── app.js                  # JavaScript 로직
│   └── styles.css              # 스타일시트
├── build/                      # 빌드 출력 (WASM + JS)
├── docs/                       # 문서
│   ├── SETUP.md                # 환경 설정 가이드
│   └── DEVELOPMENT.md          # 개발 워크플로우
├── build.sh                    # 빌드 스크립트
├── serve.sh                    # 개발 서버 스크립트
├── dev.sh                      # 통합 개발 스크립트
└── README.md                   # 이 파일
```

---

## 📚 문서

### 초보자 가이드
- **[SETUP.md](./docs/SETUP.md)**: WebAssembly 개발 환경 설정 (처음 시작하는 분 필독)
  - Emscripten 설치 및 설정
  - 개발 도구 구성
  - 문제 해결 가이드

### 개발자 가이드
- **[DEVELOPMENT.md](./docs/DEVELOPMENT.md)**: 실전 개발 워크플로우
  - C++ 필터 구현 방법
  - JavaScript 바인딩
  - 웹 인터페이스 구현
  - 빌드 및 배포
  - 성능 최적화
  - 디버깅 기법

---

## 🛠️ 기술 스택

### 백엔드 (이미지 처리)
- **C++20**: 필터 알고리즘 구현
- **Emscripten**: C++ → WebAssembly 컴파일러
- **WebAssembly**: 브라우저에서 실행되는 바이너리

### 프론트엔드
- **Vanilla JavaScript**: 웹캠 제어 및 WASM 통합
- **HTML5 Canvas**: 이미지 렌더링
- **WebRTC**: 웹캠 스트림 획득

### 빌드 도구
- **Emscripten Compiler (emcc)**: WASM 빌드
- **Bash Scripts**: 빌드 자동화
- **Python HTTP Server**: 로컬 개발 서버

---

## 🎯 성능 벤치마크

### 테스트 환경
- **해상도**: 640 × 480 (VGA)
- **프레임레이트**: 30 FPS
- **브라우저**: Chrome 120+
- **시스템**: MacBook Pro M1, 16GB RAM

### 결과 (프레임당 처리 시간)

| 필터 | 평균 시간 | 최대 시간 | FPS |
|------|----------|----------|-----|
| 원본 (미적용) | 0.1 ms | 0.3 ms | 60+ |
| 흑백 | 2.5 ms | 4.2 ms | 60+ |
| 블러 (radius=3) | 18.7 ms | 25.3 ms | 40+ |
| 엣지 검출 | 12.4 ms | 16.8 ms | 50+ |

**목표**: 33ms 이하 (30 FPS 유지) ✅

---

## 🔬 필터 알고리즘 설명

### 1. 흑백 (Grayscale)
**원리**: ITU-R BT.709 표준 색공간 변환

```
밝기(Y) = 0.2126 × R + 0.7152 × G + 0.0722 × B
```

인간의 눈이 녹색에 가장 민감하므로 가중치가 다릅니다.

### 2. 블러 (Box Blur)
**원리**: 주변 픽셀의 평균값 계산

```
픽셀(x, y) = Σ 주변 픽셀 / 픽셀 개수
```

반경(radius)이 클수록 블러 효과가 강하지만 연산량 증가 (O(r²)).

### 3. 엣지 검출 (Sobel)
**원리**: 밝기 변화율 계산 (그래디언트)

```
Gx = Sobel X 커널 적용 (수평 엣지)
Gy = Sobel Y 커널 적용 (수직 엣지)
엣지 강도 = √(Gx² + Gy²)
```

경계선을 강조하여 윤곽 추출.

---

## 🚧 개발 로드맵

### Phase 1: 기본 기능 ✅
- [x] Emscripten 환경 설정
- [x] 흑백 필터 구현
- [x] 블러 필터 구현
- [x] 엣지 검출 필터 구현
- [x] 실시간 웹캠 연동
- [x] 성능 모니터링

### Phase 2: 추가 필터 🚧
- [ ] 세피아 (Sepia)
- [ ] 반전 (Invert)
- [ ] 선명도 (Sharpen)
- [ ] 모자이크 (Pixelate)
- [ ] 색상 교체 (Color Swap)

### Phase 3: 고급 기능 📅
- [ ] 필터 강도 조절 슬라이더
- [ ] 여러 필터 조합 (Chain)
- [ ] 사진 캡처 및 다운로드
- [ ] 비디오 녹화 기능
- [ ] 필터 프리셋 저장/로드

### Phase 4: 성능 최적화 📅
- [ ] SIMD 명령어 활용
- [ ] Web Workers 멀티스레딩
- [ ] GPU 가속 (WebGL)
- [ ] 적응형 해상도 조정

### Phase 5: AI/ML 통합 🔮
- [ ] OpenCV.js 통합
- [ ] 얼굴 인식 및 트래킹
- [ ] 배경 제거 (Segmentation)
- [ ] 실시간 AR 효과

---

## 🤝 기여 가이드

### 버그 리포트
이슈를 열어주세요:
- 재현 단계
- 예상 동작 vs 실제 동작
- 브라우저 및 OS 정보
- 콘솔 에러 메시지

### 새로운 필터 추가
1. `src/filters/` 디렉토리에 `.cpp` 파일 생성
2. 필터 함수 구현 및 `EMSCRIPTEN_BINDINGS` 추가
3. `build.sh`에 파일 경로 추가
4. `web/app.js`에 JavaScript 연동 코드 추가
5. 테스트 및 성능 측정

---

## 📝 라이선스

MIT License - 자유롭게 사용, 수정, 배포 가능합니다.

---

## 🙏 감사의 글

이 프로젝트는 다음 오픈소스 프로젝트를 사용합니다:

- [Emscripten](https://emscripten.org/): C/C++ to WebAssembly 컴파일러
- [WebAssembly](https://webassembly.org/): 웹 표준 바이너리 포맷
- [MDN Web Docs](https://developer.mozilla.org/): 웹 기술 문서

---

## 📞 연락처

- **프로젝트 문의**: [GitHub Issues](../../issues)
- **버그 리포트**: [GitHub Issues](../../issues)
- **기능 제안**: [GitHub Discussions](../../discussions)

---

## 📖 참고 자료

### WebAssembly 학습
- [WebAssembly 공식 사이트](https://webassembly.org/)
- [Emscripten 공식 문서](https://emscripten.org/docs/)
- [MDN WebAssembly 가이드](https://developer.mozilla.org/en-US/docs/WebAssembly)

### 이미지 처리
- [Digital Image Processing (DIP)](https://www.tutorialspoint.com/dip/index.htm)
- [OpenCV 튜토리얼](https://docs.opencv.org/master/d9/df8/tutorial_root.html)

### 성능 최적화
- [WebAssembly Performance Tips](https://web.dev/webassembly/)
- [Emscripten Optimization](https://emscripten.org/docs/optimizing/Optimizing-Code.html)

---

**🚀 Happy Coding!**

*WebAssembly의 힘으로 웹에서도 네이티브급 성능을!*
