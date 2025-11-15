#!/bin/bash

# WebCam Filter WASM - 개발 서버 실행 스크립트
# Python 내장 HTTP 서버를 사용하여 로컬 서버를 실행합니다.

set -e  # 오류 발생 시 스크립트 중단

echo "======================================"
echo "  WebCam Filter WASM - 서버 실행"
echo "======================================"

# 1. build 디렉토리 확인
if [ ! -d "build" ]; then
    echo "❌ 오류: build/ 디렉토리가 없습니다."
    echo "먼저 빌드를 실행하세요:"
    echo "  ./build.sh"
    exit 1
fi

# 2. 필수 파일 확인
if [ ! -f "build/filters.wasm" ]; then
    echo "❌ 오류: build/filters.wasm 파일이 없습니다."
    echo "먼저 빌드를 실행하세요:"
    echo "  ./build.sh"
    exit 1
fi

# 3. Python 버전 확인
if ! command -v python3 &> /dev/null; then
    echo "❌ 오류: python3를 찾을 수 없습니다."
    exit 1
fi

echo "✅ 빌드 파일 확인 완료"
echo ""
echo "🚀 서버 실행 중..."
echo ""
echo "======================================"
echo "  브라우저에서 접속하세요:"
echo "  http://localhost:8080"
echo "======================================"
echo ""
echo "종료하려면 Ctrl+C를 누르세요."
echo ""

# 4. HTTP 서버 실행
cd build
python3 -m http.server 8080
