# 시작하기 가이드

> 완전 초보자를 위한 0부터 시작하는 개발 환경 설정 가이드

## 목차
- [필요한 사전 지식](#필요한-사전-지식)
- [시스템 요구사항](#시스템-요구사항)
- [설치 가이드](#설치-가이드)
  - [macOS](#macos)
  - [Windows](#windows)
  - [Linux](#linux)
- [프로젝트 클론 및 설정](#프로젝트-클론-및-설정)
- [첫 번째 빌드](#첫-번째-빌드)
- [개발 서버 실행](#개발-서버-실행)
- [문제 해결](#문제-해결)

---

## 필요한 사전 지식

### 필수 지식
- ✅ 터미널/명령 프롬프트 기본 사용법
- ✅ 텍스트 에디터 사용법

### 선택 사항 (있으면 좋음)
- 📚 HTML/JavaScript 기초
- 📚 C 언어 기초 (C++ 유사)
- 📚 Git 기본 명령어

---

## 시스템 요구사항

### 하드웨어
- **CPU**: 듀얼코어 이상
- **RAM**: 4GB 이상 (8GB 권장)
- **저장공간**: 5GB 이상 여유 공간
- **웹캠**: 테스트용 웹캠 (필수)

### 소프트웨어
- **운영체제**: macOS 10.14+, Windows 10+, Ubuntu 18.04+
- **인터넷**: 설치 패키지 다운로드용

---

## 설치 가이드

### macOS

#### 1. Homebrew 설치 (패키지 관리자)

```bash
# Homebrew가 이미 설치되어 있는지 확인
brew --version

# 없다면 설치
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

**예상 소요 시간**: 5~10분

#### 2. Emscripten 설치

```bash
# Emscripten SDK 다운로드
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# 최신 버전 설치
./emsdk install latest

# 최신 버전 활성화
./emsdk activate latest

# 환경 변수 설정
source ./emsdk_env.sh
```

**중요**: 터미널을 새로 열 때마다 `source ./emsdk_env.sh` 실행 필요!

**영구 설정 방법**:
```bash
# .zshrc 또는 .bash_profile에 추가
echo 'source "/경로/to/emsdk/emsdk_env.sh"' >> ~/.zshrc

# 예시:
# echo 'source "$HOME/emsdk/emsdk_env.sh"' >> ~/.zshrc
```

#### 3. 설치 확인

```bash
# Emscripten 컴파일러 확인
emcc --version
# 출력 예: emcc (Emscripten gcc/clang-like replacement) 3.1.46

em++ --version
# 출력 예: em++ (Emscripten gcc/clang-like replacement) 3.1.46
```

---

### Windows

#### 1. Git 설치

1. [Git for Windows](https://git-scm.com/download/win) 다운로드
2. 설치 파일 실행
3. 기본 옵션으로 설치

**설치 확인**:
```powershell
git --version
# 출력 예: git version 2.42.0.windows.1
```

#### 2. Python 설치 (Emscripten 요구사항)

1. [Python 다운로드](https://www.python.org/downloads/) (3.8 이상)
2. 설치 중 "Add Python to PATH" 체크 ✅

**설치 확인**:
```powershell
python --version
# 출력 예: Python 3.11.5
```

#### 3. Emscripten 설치

**PowerShell 또는 Git Bash에서 실행**:

```powershell
# Emscripten SDK 다운로드
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# 최신 버전 설치
.\emsdk install latest

# 최신 버전 활성화
.\emsdk activate latest

# 환경 변수 설정 (현재 세션)
.\emsdk_env.bat
```

**영구 환경 변수 설정**:
1. `시스템 환경 변수 편집` 검색
2. `환경 변수` 버튼 클릭
3. `Path` 선택 → `편집`
4. `새로 만들기` → `C:\경로\to\emsdk` 추가
5. `C:\경로\to\emsdk\upstream\emscripten` 추가

#### 4. 설치 확인

```powershell
emcc --version
em++ --version
python --version
```

---

### Linux (Ubuntu/Debian)

#### 1. 필수 패키지 설치

```bash
# 패키지 목록 업데이트
sudo apt update

# Git 설치
sudo apt install git

# Python 설치 (대부분 기본 설치됨)
sudo apt install python3 python3-pip

# 빌드 도구 설치
sudo apt install build-essential
```

#### 2. Emscripten 설치

```bash
# Emscripten SDK 다운로드
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# 최신 버전 설치
./emsdk install latest

# 최신 버전 활성화
./emsdk activate latest

# 환경 변수 설정
source ./emsdk_env.sh
```

**영구 설정**:
```bash
# .bashrc에 추가
echo 'source "$HOME/emsdk/emsdk_env.sh"' >> ~/.bashrc

# 적용
source ~/.bashrc
```

#### 3. 설치 확인

```bash
emcc --version
em++ --version
python3 --version
```

---

## 프로젝트 클론 및 설정

### 1. 프로젝트 다운로드

```bash
# 프로젝트 클론
git clone https://github.com/yourusername/webcam-filter-wasm.git

# 프로젝트 디렉토리로 이동
cd webcam-filter-wasm

# 프로젝트 구조 확인
ls -la
```

**예상 출력**:
```
.
├── README.md
├── docs/
├── web/
│   ├── index.html
│   ├── styles.css
│   ├── app.js
│   └── wrapper.js
├── src/
│   └── filters/
│       └── grayscale.cpp
├── build.sh
└── serve.sh
```

**참고**: `build.sh` 스크립트가 빌드 디렉토리를 자동으로 생성하므로 수동으로 생성할 필요 없습니다.

---

## 첫 번째 빌드

### 빌드 스크립트 실행

#### macOS/Linux

```bash
# 빌드 스크립트 실행 권한 부여
chmod +x build.sh

# 빌드 실행
./build.sh
```

#### Windows (Git Bash 또는 PowerShell)

```bash
# Git Bash에서 실행 (권장)
./build.sh

# 또는 PowerShell에서 수동 빌드
emcc src/filters/grayscale.cpp -o build/filters.js -O3 --bind -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s MODULARIZE=1 -s EXPORT_NAME="Module" -s EXPORTED_RUNTIME_METHODS='["cwrap","ccall"]'
```

### 빌드 과정 이해

```
1. Emscripten 환경 확인
   - emcc 명령어 사용 가능 여부 체크
   - 버전 정보 출력

2. 빌드 디렉토리 생성
   - build/ 디렉토리 자동 생성

3. C++ → WebAssembly 컴파일
   - src/filters/grayscale.cpp → build/filters.wasm
   - JavaScript 래퍼 생성 → build/filters.js

4. 웹 파일 복사
   - web/ 디렉토리의 HTML, CSS, JS를 build/로 복사

5. 출력 파일
   - build/filters.js
   - build/filters.wasm
   - build/index.html
   - build/styles.css
   - build/app.js
   - build/wrapper.js
```

### 예상 빌드 시간

- **첫 빌드**: 30초 ~ 1분
- **이후 빌드**: 5초 ~ 10초

### 빌드 성공 확인

```bash
# 생성된 파일 확인
ls -lh build/

# 예상 출력:
# -rw-r--r--  filters.js    (약 50KB)
# -rw-r--r--  filters.wasm  (약 20KB)
# -rw-r--r--  index.html
# -rw-r--r--  styles.css
# -rw-r--r--  app.js
# -rw-r--r--  wrapper.js
```

---

## 개발 서버 실행

### 방법 1: serve.sh 스크립트 사용 (권장)

```bash
# 프로젝트 루트에서 실행
./serve.sh
```

**출력 예시**:
```
======================================
  WebCam Filter WASM - 개발 서버
======================================

✅ 서버 실행 중...

브라우저에서 접속:
  http://localhost:8080

종료: Ctrl+C
======================================

Serving HTTP on 0.0.0.0 port 8080 (http://0.0.0.0:8080/) ...
```

### 방법 2: Python 내장 서버

```bash
# 빌드 디렉토리로 이동하여 실행
cd build
python3 -m http.server 8080

# 또는 Python 2
python -m SimpleHTTPServer 8080
```

### 방법 3: VS Code Live Server

1. VS Code 설치
2. Live Server 확장 프로그램 설치
3. `build/index.html` 우클릭
4. "Open with Live Server" 선택

### 브라우저에서 확인

1. 브라우저 열기 (Chrome 권장)
2. 주소창에 입력: `http://localhost:8080`
3. 웹캠 권한 허용
4. 필터 버튼 클릭하여 테스트

**예상 화면**:
```
┌───────────────────────────────────┐
│  Webcam Filter (WASM)             │
├───────────────────────────────────┤
│                                   │
│     [웹캠 영상 표시]               │
│                                   │
├───────────────────────────────────┤
│  [흑백 필터]  [좌우반전]          │
└───────────────────────────────────┘
```

---

## 문제 해결

### 1. Emscripten 명령어를 찾을 수 없음

**증상**:
```bash
emcc: command not found
```

**해결**:
```bash
# 환경 변수 다시 설정
source ~/emsdk/emsdk_env.sh  # macOS/Linux
# 또는
.\emsdk_env.bat              # Windows

# 영구 설정 확인
cat ~/.zshrc                 # macOS
cat ~/.bashrc                # Linux
```

### 2. 빌드 실패 - 권한 오류

**증상**:
```bash
Permission denied: ./build.sh
```

**해결**:
```bash
chmod +x build.sh
./build.sh
```

### 4. 웹캠이 작동하지 않음

**증상**: 검은 화면만 표시

**해결**:
1. **HTTPS 확인**: 로컬 서버는 localhost이므로 OK
2. **브라우저 권한**: 설정 → 사이트 설정 → 카메라 → 허용
3. **다른 앱 확인**: 다른 프로그램이 웹캠 사용 중인지 확인
4. **브라우저 재시작**: 캐시 삭제 후 재시작

### 5. WASM 파일 로딩 실패

**증상**:
```
CompileError: WebAssembly.instantiate(): Wasm code generation disallowed
```

**해결**:
```bash
# 로컬 서버 실행 확인 (file:// 프로토콜은 안됨)
# http://localhost:8000 사용 필수
```

### 6. 빌드는 성공했지만 화면이 안 나옴

**체크리스트**:
```bash
# 1. WASM 파일 생성 확인
ls -lh build/filters.wasm

# 2. JavaScript 파일 확인
ls -lh build/filters.js

# 3. 브라우저 콘솔 확인 (F12)
# - 에러 메시지 확인
# - 네트워크 탭에서 파일 로딩 확인

# 4. 서버 로그 확인
# Python 서버 출력에서 404 에러 없는지 확인
```

---

## 다음 단계

환경 설정을 완료했다면:

1. 🏗️ [프로젝트 구조 이해하기](ARCHITECTURE.md)
   - 각 파일의 역할 파악
   - 데이터 흐름 이해

2. 🎓 [튜토리얼 시작하기](TUTORIAL.md)
   - 첫 번째 필터 만들기
   - 코드 수정 및 테스트

3. 📚 [API 레퍼런스](API_REFERENCE.md)
   - 사용 가능한 함수 목록
   - 매개변수 상세 설명

---

## 개발 환경 최적화

### 추천 개발 도구

#### 코드 에디터
- **Visual Studio Code** (권장)
  - C/C++ 확장 프로그램
  - Live Server 확장

- **CLion** (전문가용)
  - 강력한 C++ 지원
  - WebAssembly 디버깅 지원

#### 브라우저
- **Google Chrome** (권장)
  - 강력한 개발자 도구
  - WebAssembly 디버깅 지원

### 개발 워크플로우

```
1. 코드 수정 (src/filters/grayscale.cpp)
   ↓
2. 빌드 (./build.sh)
   ↓
3. 브라우저 새로고침 (F5)
   ↓
4. 테스트 및 확인
   ↓
5. 반복
```

### 유용한 단축키

```bash
# 빠른 빌드 + 테스트
./build.sh && echo "Build OK"

# 빌드 후 자동 브라우저 열기 (macOS)
./build.sh && open http://localhost:8080

# 파일 변경 감지 자동 빌드 (watch 설치 필요)
# macOS: brew install watch
watch -n 2 ./build.sh
```

---

## 추가 리소스

### 공식 문서
- [Emscripten 튜토리얼](https://emscripten.org/docs/getting_started/Tutorial.html)
- [WebAssembly MDN](https://developer.mozilla.org/en-US/docs/WebAssembly)
- [C++ 레퍼런스](https://en.cppreference.com/)

### 커뮤니티
- [Emscripten GitHub](https://github.com/emscripten-core/emscripten)
- [WebAssembly Discord](https://discord.gg/webassembly)
- [Stack Overflow](https://stackoverflow.com/questions/tagged/webassembly)

### 학습 자료
- [Learn C++](https://www.learncpp.com/)
- [WebAssembly 기초](https://www.youtube.com/results?search_query=webassembly+tutorial)
- [Emscripten 예제](https://github.com/emscripten-core/emscripten/tree/main/tests)

---

## 체크리스트

설정 완료 확인:

- [ ] Emscripten 설치 완료 (`emcc --version` 실행됨)
- [ ] 프로젝트 클론 완료
- [ ] 빌드 성공 (`./build.sh` 실행 → `build/filters.js`, `build/filters.wasm` 생성)
- [ ] 로컬 서버 실행 가능 (`./serve.sh` 또는 `python3 -m http.server`)
- [ ] 브라우저에서 웹캠 작동 확인 (`http://localhost:8080`)
- [ ] 필터 적용 정상 작동 (흑백 필터, 좌우반전 버튼 동작)

모두 체크했다면 축하합니다! 🎉 이제 개발을 시작할 준비가 되었습니다.
