# WebAssembly 개발 환경 설정 가이드

> **대상**: WebAssembly를 처음 시작하는 개발자 (Java/Spring Boot 경험자 기준)

이 문서는 실시간 웹캠 필터 프로젝트를 위한 개발 환경을 처음부터 설정하는 방법을 안내합니다.

---

## 📋 목차

1. [개요](#개요)
2. [사전 요구사항](#사전-요구사항)
3. [Emscripten 설치](#emscripten-설치)
4. [프로젝트 구조 이해](#프로젝트-구조-이해)
5. [개발 도구 설정](#개발-도구-설정)
6. [첫 번째 빌드](#첫-번째-빌드)
7. [문제 해결](#문제-해결)

---

## 개요

### WebAssembly란?

- **WebAssembly (WASM)**: 웹 브라우저에서 실행되는 저수준 바이너리 포맷
- **Spring Boot와 비교**:
  - Spring Boot: JVM 위에서 실행되는 Java 바이트코드
  - WebAssembly: 브라우저에서 실행되는 네이티브급 성능의 바이너리 코드
- **사용 사례**: 이미지/비디오 처리, 게임, 암호화 등 성능이 중요한 작업

### Emscripten이란?

- **역할**: C/C++ 코드를 WebAssembly로 컴파일하는 도구
- **Java 개발자 관점**: `javac`가 `.java` → `.class`로 컴파일하듯, `emcc`는 `.cpp` → `.wasm`으로 컴파일
- **추가 기능**: JavaScript 바인딩 자동 생성, HTML 템플릿 제공

---

## 사전 요구사항

### macOS 시스템 요구사항

```bash
# 1. Xcode Command Line Tools 설치 확인
xcode-select --install

# 2. Homebrew 설치 확인 (선택사항, 권장)
brew --version

# 3. Git 설치 확인
git --version

# 4. Python 3.x 설치 확인 (Emscripten 필수)
python3 --version
```

**각 도구의 역할**:
- **Xcode CLI Tools**: C++ 컴파일러 및 기본 빌드 도구
- **Git**: Emscripten 소스 코드 다운로드
- **Python**: Emscripten 빌드 스크립트 실행

---

## Emscripten 설치

### Step 1: Emscripten SDK 다운로드

```bash
# 프로젝트 루트 디렉토리로 이동
cd ~/CLionProjects/webcam-filter-wasm

# Emscripten SDK 클론 (이미 emsdk 폴더가 있다면 스킵)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
```

**Java 비유**: Maven이나 Gradle을 설치하는 것과 유사합니다.

### Step 2: 최신 버전 설치 및 활성화

```bash
# 1. 최신 stable 버전 설치
./emsdk install latest

# 2. 설치한 버전 활성화
./emsdk activate latest

# 3. 환경 변수 설정 (현재 터미널 세션에만 적용)
source ./emsdk_env.sh
```

**각 명령어 설명**:
- `install`: SDK 다운로드 및 설치 (약 5-10분 소요)
- `activate`: 설치된 버전을 활성화
- `source`: 환경 변수를 현재 셸에 적용

### Step 3: 영구 환경 변수 설정 (선택사항)

매번 `source emsdk_env.sh`를 실행하기 번거롭다면:

```bash
# zsh 사용자 (macOS 기본)
echo 'source ~/CLionProjects/webcam-filter-wasm/emsdk/emsdk_env.sh > /dev/null 2>&1' >> ~/.zshrc

# bash 사용자
echo 'source ~/CLionProjects/webcam-filter-wasm/emsdk/emsdk_env.sh > /dev/null 2>&1' >> ~/.bash_profile
```

**Java 비유**: `JAVA_HOME`, `MAVEN_HOME`을 `.bashrc`에 추가하는 것과 동일합니다.

### Step 4: 설치 확인

```bash
# Emscripten 컴파일러 버전 확인
emcc --version

# 예상 출력:
# emscripten 3.1.xx (yyyy-mm-dd)
```

---

## 프로젝트 구조 이해

### 권장 디렉토리 구조

```
webcam-filter-wasm/
├── emsdk/                    # Emscripten SDK (설치됨)
├── src/                      # C++ 소스 코드
│   ├── filters/              # 필터 구현 (흑백, 블러 등)
│   │   ├── grayscale.cpp
│   │   ├── blur.cpp
│   │   └── edge.cpp
│   └── main.cpp              # 메인 진입점 + JavaScript 바인딩
├── web/                      # 웹 프론트엔드
│   ├── index.html            # 메인 HTML
│   ├── app.js                # JavaScript 로직
│   └── styles.css            # 스타일시트
├── build/                    # 빌드 출력 (.wasm, .js)
├── docs/                     # 문서
│   ├── SETUP.md              # 이 파일
│   └── DEVELOPMENT.md        # 개발 워크플로우
└── README.md                 # 프로젝트 소개
```

**Spring Boot 프로젝트와 비교**:

| WebAssembly 프로젝트 | Spring Boot 프로젝트 |
|---------------------|---------------------|
| `src/` (C++)        | `src/main/java`     |
| `web/` (HTML/JS)    | `src/main/resources/static` |
| `build/` (WASM)     | `target/` (JAR)     |

---

## 개발 도구 설정

### CLion 설정 (이미 사용 중)

CLion은 C++ IDE로 훌륭하지만, WebAssembly 빌드는 별도 명령어가 필요합니다.

**권장 설정**:
1. **External Tools 추가** (옵션):
   - `Preferences` → `Tools` → `External Tools` → `+`
   - **Name**: `Build WASM`
   - **Program**: `$ProjectFileDir$/build.sh`
   - **Working directory**: `$ProjectFileDir$`

### 웹 서버 설치 (개발용)

WASM 파일은 로컬 파일(`file://`)로는 실행 불가능합니다. 간단한 HTTP 서버가 필요합니다.

```bash
# Python 내장 서버 사용 (가장 간단)
# 실행은 나중에 build/ 디렉토리에서 진행
python3 -m http.server 8080

# 또는 Node.js 사용자라면
npm install -g http-server
http-server build/ -p 8080
```

**Java 비유**: Spring Boot의 내장 Tomcat과 유사한 역할입니다.

---

## 첫 번째 빌드

### Hello World 컴파일

```bash
# 프로젝트 루트로 이동
cd ~/CLionProjects/webcam-filter-wasm

# 간단한 테스트 빌드
emcc main.cpp -o hello.html

# 생성된 파일 확인
ls -lh hello.*
# hello.html  - 테스트용 HTML 페이지
# hello.js    - JavaScript 바인딩 코드
# hello.wasm  - WebAssembly 바이너리
```

**각 파일의 역할**:
- **`.wasm`**: 실제 C++ 로직이 컴파일된 바이너리 (`.class` 파일과 유사)
- **`.js`**: WASM을 로드하고 JavaScript와 연결하는 글루 코드
- **`.html`**: 테스트용 실행 페이지 (자동 생성)

### 웹 브라우저에서 실행

```bash
# 간단한 웹 서버 실행
python3 -m http.server 8080

# 브라우저에서 접속
# http://localhost:8080/hello.html
```

**예상 결과**: 브라우저 콘솔에 "Hello and welcome to C++!" 출력

---

## 문제 해결

### 1. `emcc: command not found`

**원인**: 환경 변수가 설정되지 않음

**해결**:
```bash
cd ~/CLionProjects/webcam-filter-wasm/emsdk
source ./emsdk_env.sh
```

### 2. `Python 2.x is not supported`

**원인**: Emscripten은 Python 3.x 필요

**해결**:
```bash
# Homebrew로 Python 3 설치
brew install python3

# 버전 확인
python3 --version
```

### 3. WASM 파일이 브라우저에서 로드 안 됨

**원인**: CORS 정책 또는 `file://` 프로토콜 사용

**해결**: 반드시 HTTP 서버를 통해 접근
```bash
python3 -m http.server 8080
# http://localhost:8080/hello.html
```

### 4. macOS Catalina 이상에서 "cannot be opened" 경고

**원인**: Gatekeeper 보안 정책

**해결**:
```bash
# Emscripten 바이너리에 실행 권한 부여
cd ~/CLionProjects/webcam-filter-wasm/emsdk
chmod +x ./emsdk
```

---

## 다음 단계

환경 설정이 완료되었습니다! 이제 다음 문서를 확인하세요:

- **[DEVELOPMENT.md](./DEVELOPMENT.md)**: 실제 웹캠 필터 개발 워크플로우
- **[README.md](../README.md)**: 프로젝트 전체 개요

---

## 참고 자료

### 공식 문서
- [Emscripten 공식 문서](https://emscripten.org/docs/getting_started/index.html)
- [WebAssembly 공식 사이트](https://webassembly.org/)
- [MDN WebAssembly 가이드](https://developer.mozilla.org/ko/docs/WebAssembly)

### 개념 이해
- [Java 개발자를 위한 WebAssembly 소개](https://www.infoq.com/articles/webassembly-introduction/)
- [WebAssembly vs JavaScript 성능 비교](https://hacks.mozilla.org/2017/02/a-cartoon-intro-to-webassembly/)

---

**작성일**: 2025-11-08
**대상 독자**: WebAssembly 초보자 (Java/Spring Boot 경험자)