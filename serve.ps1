# WebCam Filter WASM - 개발 서버 실행 스크립트 (PowerShell)
# Python 내장 HTTP 서버를 사용하여 로컬 서버를 실행합니다.

# UTF-8 인코딩 설정 (한글 깨짐 방지)
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

# 에러 발생 시 스크립트 중단하지만 에러 메시지는 표시
$ErrorActionPreference = "Stop"

try {
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "  WebCam Filter WASM - 서버 실행" -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host ""

    # 1. build 디렉토리 확인
    if (!(Test-Path "build")) {
        Write-Host ""
        Write-Host "❌ 오류: build/ 디렉토리가 없습니다." -ForegroundColor Red
        Write-Host ""
        Write-Host "먼저 빌드를 실행하세요:" -ForegroundColor Yellow
        Write-Host "  .\build.ps1" -ForegroundColor Yellow
        Write-Host "  또는 Git Bash에서: ./build.sh" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Press any key to exit..." -ForegroundColor Gray
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        exit 1
    }

    # 2. 필수 파일 확인
    if (!(Test-Path "build/filters.wasm")) {
        Write-Host ""
        Write-Host "❌ 오류: build/filters.wasm 파일이 없습니다." -ForegroundColor Red
        Write-Host ""
        Write-Host "먼저 빌드를 실행하세요:" -ForegroundColor Yellow
        Write-Host "  .\build.ps1" -ForegroundColor Yellow
        Write-Host "  또는 Git Bash에서: ./build.sh" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Press any key to exit..." -ForegroundColor Gray
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        exit 1
    }

    # 3. Python 확인
    $pythonCmd = $null
    if (Get-Command python3 -ErrorAction SilentlyContinue) {
        $pythonCmd = "python3"
        Write-Host "✅ Python 발견: python3" -ForegroundColor Green
    } elseif (Get-Command python -ErrorAction SilentlyContinue) {
        $pythonCmd = "python"
        Write-Host "✅ Python 발견: python" -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "❌ 오류: Python을 찾을 수 없습니다." -ForegroundColor Red
        Write-Host ""
        Write-Host "Python을 설치하세요:" -ForegroundColor Yellow
        Write-Host "  https://www.python.org/downloads/" -ForegroundColor Cyan
        Write-Host ""
        Write-Host "설치 후 PowerShell을 다시 시작하세요." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Press any key to exit..." -ForegroundColor Gray
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
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

} catch {
    # 예상치 못한 에러 발생 시
    Write-Host ""
    Write-Host "======================================" -ForegroundColor Red
    Write-Host "  ❌ 에러 발생!" -ForegroundColor Red
    Write-Host "======================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "에러 메시지:" -ForegroundColor Yellow
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    Write-Host "에러 위치:" -ForegroundColor Yellow
    Write-Host $_.InvocationInfo.PositionMessage -ForegroundColor Gray
    Write-Host ""
    Write-Host "======================================" -ForegroundColor Red
    Write-Host ""
    Write-Host "Press any key to exit..." -ForegroundColor Gray
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit 1
}
