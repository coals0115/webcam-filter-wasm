#!/bin/bash

# WebCam Filter WASM - 통합 개발 스크립트
# 빌드 + 서버 실행을 한 번에 수행합니다.

set -e  # 오류 발생 시 스크립트 중단

echo "======================================"
echo "  WebCam Filter WASM - 개발 모드"
echo "======================================"

# 1. 빌드 실행
echo ""
echo "1️⃣ 프로젝트 빌드 중..."
./build.sh

# 2. 서버 실행
echo ""
echo "2️⃣ 개발 서버 실행 중..."
./serve.sh
