#!/bin/bash

# WebCam Filter WASM - 빌드 스크립트
# C++ 소스를 WebAssembly로 컴파일합니다.

set -e  # 오류 발생 시 스크립트 중단

echo "======================================"
echo "  WebCam Filter WASM - 빌드 시작"
echo "======================================"

# 1. Emscripten 환경 확인
if ! command -v emcc &> /dev/null; then
    echo "❌ 오류: emcc를 찾을 수 없습니다."
    echo "다음 명령어를 실행하세요:"
    echo "  cd emsdk && source ./emsdk_env.sh && cd .."
    exit 1
fi

echo "✅ Emscripten 버전: $(emcc --version | head -n 1)"

# 2. 빌드 디렉토리 생성
echo ""
echo "📁 빌드 디렉토리 생성 중..."
mkdir -p build
echo "✅ build/ 디렉토리 준비 완료"

# 3. C++ 소스 컴파일
echo ""
echo "🔨 C++ → WebAssembly 컴파일 중..."
emcc \
    src/filters/grayscale.cpp \
    -o build/filters.js \
    -O3 \
    --bind \
    -s WASM=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s MODULARIZE=1 \
    -s EXPORT_NAME="Module" \
    -s EXPORTED_RUNTIME_METHODS='["cwrap","ccall"]'

echo "✅ 컴파일 완료: build/filters.wasm, build/filters.js"

# 4. 웹 파일 복사
echo ""
echo "📋 웹 파일 복사 중..."
cp web/index.html build/
cp web/styles.css build/
cp web/app.js build/
cp web/wrapper.js build/
echo "✅ 웹 파일 복사 완료"

# 5. 빌드 결과 확인
echo ""
echo "======================================"
echo "  빌드 완료! 🎉"
echo "======================================"
echo ""
echo "생성된 파일:"
ls -lh build/
echo ""
echo "실행 방법:"
echo "  ./serve.sh"
echo "  또는"
echo "  cd build && python3 -m http.server 8080"
echo ""
echo "브라우저에서 접속:"
echo "  http://localhost:8080"
echo ""
