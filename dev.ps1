# WebCam Filter WASM - 통합 개발 스크립트 (PowerShell)
# 빌드 + 서버 실행을 한 번에 수행합니다.

# 에러 발생 시 스크립트 중단하지만 에러 메시지는 표시
$ErrorActionPreference = "Stop"

try {
    Write-Host "======================================" -ForegroundColor Cyan
    Write-Host "  WebCam Filter WASM - 개발 모드" -ForegroundColor Cyan
    Write-Host "======================================" -ForegroundColor Cyan

    # 1. 빌드 실행
    Write-Host ""
    Write-Host "1️⃣ 프로젝트 빌드 중..." -ForegroundColor Cyan
    & .\build.ps1

    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "❌ 빌드 실패" -ForegroundColor Red
        Write-Host ""
        Write-Host "Press any key to exit..." -ForegroundColor Gray
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
        exit 1
    }

    # 2. 서버 실행
    Write-Host ""
    Write-Host "2️⃣ 개발 서버 실행 중..." -ForegroundColor Cyan
    & .\serve.ps1

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
