# WebCam Filter WASM - 개발 서버 실행 스크립트 (PowerShell)
# Python 내장 HTTP 서버를 사용하여 로컬 서버를 실행합니다.

$ErrorActionPreference = "Stop"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  WebCam Filter WASM - 서버 실행" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# 1. build 디렉토리 확인
if (!(Test-Path "build")) {
    Write-Host "❌ 오류: build/ 디렉토리가 없습니다." -ForegroundColor Red
    Write-Host "먼저 빌드를 실행하세요:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1" -ForegroundColor Yellow
    Write-Host "  또는 Git Bash에서: ./build.sh" -ForegroundColor Yellow
    exit 1
}

# 2. 필수 파일 확인
if (!(Test-Path "build/filters.wasm")) {
    Write-Host "❌ 오류: build/filters.wasm 파일이 없습니다." -ForegroundColor Red
    Write-Host "먼저 빌드를 실행하세요:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1" -ForegroundColor Yellow
    Write-Host "  또는 Git Bash에서: ./build.sh" -ForegroundColor Yellow
    exit 1
}

# 3. Python 확인
$pythonCmd = $null
if (Get-Command python3 -ErrorAction SilentlyContinue) {
    $pythonCmd = "python3"
} elseif (Get-Command python -ErrorAction SilentlyContinue) {
    $pythonCmd = "python"
} else {
    Write-Host "❌ 오류: Python을 찾을 수 없습니다." -ForegroundColor Red
    Write-Host "Python을 설치하세요: https://www.python.org/downloads/" -ForegroundColor Yellow
    exit 1
}

Write-Host "✅ 빌드 파일 확인 완료" -ForegroundColor Green
Write-Host ""
Write-Host "🚀 서버 실행 중..." -ForegroundColor Cyan
Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  브라우저에서 접속하세요:" -ForegroundColor Green
Write-Host "  http://localhost:8080" -ForegroundColor Yellow
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "종료하려면 Ctrl+C를 누르세요." -ForegroundColor Gray
Write-Host ""

# 4. HTTP 서버 실행
Set-Location build
& $pythonCmd -m http.server 8080
