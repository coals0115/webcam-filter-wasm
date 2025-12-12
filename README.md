# 8조 - WebCam Filter WASM

## 팀 정보

| 역할 | 이름 |
|------|------|
| 팀장 | 강성준 |
| 팀원 | 방채민 |
| 팀원 | 손아영 |

---

## 프로젝트 개요

WebAssembly를 활용한 실시간 웹캠 필터 애플리케이션

C++로 작성된 이미지 처리 알고리즘을 WebAssembly로 컴파일하여 브라우저에서 실시간 필터를 적용합니다.

### 주요 기능

- **6가지 필터**: Grayscale, Sepia, X-Ray, Mirror, Chroma Key, Thermal
- **실시간 처리**: 60 FPS 웹캠 영상에 필터 적용
- **크로마키**: 배경 이미지 합성 기능
- **성능 모니터링**: 처리 시간 및 FPS 실시간 표시

---

## 실행 방법

```bash
# 1. Emscripten 환경 활성화
cd emsdk && source ./emsdk_env.sh && cd ..

# 2. 빌드
./build.sh

# 3. 서버 실행
./serve.sh

# 4. 브라우저에서 접속
# http://localhost:8080
```

---

## 역할 분담

| 이름 | 담당 업무 |
|------|-----------|
| 강성준 | 필터 알고리즘 구현 (Grayscale, Mirror, Chroma Key, Thermal) |
| 손아영 | 필터 알고리즘 구현 (Sepia, X-Ray) |
| 방채민 | UI/UX, 빌드/배포, JS↔WASM 연동 |

---

## 어려웠던 점과 해결 방법

> *[TODO: 팀원들의 의견을 반영하여 작성 예정]*

---

## 가산점 항목

### 1. 추가 기능 구현
- 고급 필터 6개 구현
- 크로마키 배경 합성 시스템
- 성능 모니터링 대시보드

### 2. UI/UX 개선
- 반응형 디자인
- 웹 접근성 (ARIA 속성)

### 3. 성능 최적화
- 정수 연산 + 비트시프트 (부동소수점 연산 최소화)
- sqrt 연산 회피 (거리 계산 시 제곱 비교)
- 버퍼 재사용 (GC 부담 제거)

---

## Latency 측정

### 테스트 환경
- 해상도: 640 × 480
- 브라우저: Chrome
- 측정: 50회 평균 (`runBenchmark(50)`)

### C++ (WASM) vs JavaScript 성능 비교

| 필터 | C++ (WASM) | JavaScript | 성능 향상 |
|------|------------|------------|----------|
| Grayscale | 0.34ms | 0.48ms | **1.4x** |
| Sepia | 0.26ms | 0.75ms | **2.9x** |
| X-Ray | 0.50ms | 0.76ms | **1.5x** |
| Mirror | 0.30ms | 0.49ms | **1.6x** |
| Thermal | 0.65ms | 0.80ms | **1.2x** |

**→ 모든 필터에서 C++/WASM이 JavaScript보다 1.2x ~ 2.9x 빠름**
