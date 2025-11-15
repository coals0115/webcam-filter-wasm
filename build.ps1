# WebCam Filter WASM - 빌드 스크립트 (PowerShell)
# C++ 소스를 WebAssembly로 컴파일합니다.

$ErrorActionPreference = "Stop"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  WebCam Filter WASM - 빌드 시작" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# 1. Emscripten 환경 확인
if (!(Get-Command emcc -ErrorAction SilentlyContinue)) {
    Write-Host "❌ 오류: emcc를 찾을 수 없습니다." -ForegroundColor Red
    Write-Host "다음 명령어를 실행하세요:" -ForegroundColor Yellow
    Write-Host "  cd emsdk" -ForegroundColor Yellow
    Write-Host "  .\emsdk_env.ps1" -ForegroundColor Yellow
    Write-Host "  cd .." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "또는 Git Bash에서:" -ForegroundColor Yellow
    Write-Host "  cd emsdk && source ./emsdk_env.sh && cd .." -ForegroundColor Yellow
    exit 1
}

$emccVersion = (emcc --version 2>&1 | Select-Object -First 1)
Write-Host "✅ Emscripten 버전: $emccVersion" -ForegroundColor Green

# 2. 빌드 디렉토리 생성
Write-Host ""
Write-Host "📁 빌드 디렉토리 생성 중..." -ForegroundColor Cyan
New-Item -ItemType Directory -Force -Path build | Out-Null
Write-Host "✅ build/ 디렉토리 준비 완료" -ForegroundColor Green

# 3. C++ 소스 컴파일
Write-Host ""
Write-Host "🔨 C++ → WebAssembly 컴파일 중..." -ForegroundColor Cyan

emcc `
    src/filters/grayscale.cpp `
    -o build/filters.js `
    -O3 `
    --bind `
    -s WASM=1 `
    -s ALLOW_MEMORY_GROWTH=1 `
    -s MODULARIZE=1 `
    -s EXPORT_NAME="Module" `
    -s EXPORTED_RUNTIME_METHODS='["cwrap","ccall"]'

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ 컴파일 실패" -ForegroundColor Red
    exit 1
}

Write-Host "✅ 컴파일 완료: build/filters.wasm, build/filters.js" -ForegroundColor Green

# 4. 웹 파일 복사
Write-Host ""
Write-Host "📋 웹 파일 복사 중..." -ForegroundColor Cyan
Copy-Item web/index.html build/
Copy-Item web/styles.css build/
Copy-Item web/app.js build/
Write-Host "✅ 웹 파일 복사 완료" -ForegroundColor Green

# 5. 빌드 결과 확인
Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  빌드 완료! 🎉" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "생성된 파일:" -ForegroundColor Cyan
Get-ChildItem build/ | Format-Table Name, Length, LastWriteTime
Write-Host ""
Write-Host "실행 방법:" -ForegroundColor Cyan
Write-Host "  .\serve.ps1" -ForegroundColor Yellow
Write-Host "  또는" -ForegroundColor Gray
Write-Host "  cd build; python -m http.server 8080" -ForegroundColor Yellow
Write-Host ""
Write-Host "브라우저에서 접속:" -ForegroundColor Cyan
Write-Host "  http://localhost:8080" -ForegroundColor Yellow
Write-Host ""
