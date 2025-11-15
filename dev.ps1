# WebCam Filter WASM - 통합 개발 스크립트 (PowerShell)
# 빌드 + 서버 실행을 한 번에 수행합니다.

$ErrorActionPreference = "Stop"

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "  WebCam Filter WASM - 개발 모드" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan

# 1. 빌드 실행
Write-Host ""
Write-Host "1️⃣ 프로젝트 빌드 중..." -ForegroundColor Cyan
& .\build.ps1

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ 빌드 실패" -ForegroundColor Red
    exit 1
}

# 2. 서버 실행
Write-Host ""
Write-Host "2️⃣ 개발 서버 실행 중..." -ForegroundColor Cyan
& .\serve.ps1
