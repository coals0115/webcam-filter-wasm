# 학습 로드맵

> 단계별 학습 경로와 권장 순서

## 목차
- [학습 전 체크리스트](#학습-전-체크리스트)
- [초보자 경로 (0-2주)](#초보자-경로-0-2주)
- [중급자 경로 (2-4주)](#중급자-경로-2-4주)
- [고급자 경로 (4주+)](#고급자-경로-4주)
- [실습 프로젝트 아이디어](#실습-프로젝트-아이디어)
- [추천 학습 자료](#추천-학습-자료)

---

## 학습 전 체크리스트

### 필수 사전 지식

#### 기초 프로그래밍
- [ ] 변수, 함수, 조건문, 반복문 이해
- [ ] 배열과 객체 사용법
- [ ] 기본 디버깅 방법

#### 웹 개발 기초
- [ ] HTML 기본 태그 (`<div>`, `<button>`, `<canvas>` 등)
- [ ] CSS 기본 스타일링
- [ ] JavaScript DOM 조작

#### 개발 환경
- [ ] 터미널/명령 프롬프트 사용법
- [ ] 텍스트 에디터 (VS Code 권장)
- [ ] 브라우저 개발자 도구 (F12)

### 선택 사항 (있으면 좋음)
- 📚 Git 기본 명령어
- 📚 C 언어 기초
- 📚 이미지 처리 개념

---

## 초보자 경로 (0-2주)

> "C++과 WebAssembly를 처음 접하는 분들을 위한 기초 학습"

### Week 1: 개념 이해와 환경 설정

#### Day 1-2: 기초 개념 학습
- [ ] 📖 [CONCEPTS.md](CONCEPTS.md) 읽기
  - C++이란 무엇인가?
  - WebAssembly가 필요한 이유
  - Emscripten의 역할

**실습**:
```
✅ 핵심 개념 3줄 요약 작성해보기
✅ WebAssembly vs JavaScript 차이점 정리
```

**예상 시간**: 2-3시간

---

#### Day 3-4: 개발 환경 설정
- [ ] 📖 [GETTING_STARTED.md](GETTING_STARTED.md) 따라하기
  - Emscripten 설치
  - CMake 설치
  - 프로젝트 클론

**실습**:
```bash
# 설치 확인 명령어 실행
emcc --version
cmake --version

# 첫 번째 빌드
./build.sh
```

**예상 시간**: 3-4시간

**문제 발생 시**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md) 참조

---

#### Day 5-7: 프로젝트 구조 이해
- [ ] 📖 [ARCHITECTURE.md](ARCHITECTURE.md) 학습
  - 파일 구조 이해
  - 데이터 흐름 파악
  - 메모리 관리 개념

**실습**:
```
✅ 각 파일의 역할 직접 설명해보기
✅ 데이터 흐름도 그려보기
✅ 개발 서버 실행 및 테스트
```

**예상 시간**: 4-5시간

---

### Week 2: 첫 번째 필터 만들기

#### Day 8-10: 튜토리얼 1 - 세피아 필터
- [ ] 🎓 [TUTORIAL.md - 세피아 필터](TUTORIAL.md#튜토리얼-1-세피아-필터-만들기)
  - C++ 함수 작성
  - JavaScript 바인딩
  - UI 연결

**실습 체크리스트**:
```cpp
✅ C++에서 applySepiaFilterRaw() 함수 구현
✅ EMSCRIPTEN_BINDINGS에 함수 추가
✅ 빌드 성공 확인
✅ JavaScript에서 함수 호출
✅ HTML에 버튼 추가
✅ 브라우저에서 테스트
```

**예상 시간**: 5-6시간

**성공 기준**:
- 세피아 필터 버튼 클릭 시 정상 작동
- FPS 30 이상 유지
- 에러 없음

---

#### Day 11-14: 코드 이해 심화
- [ ] 📚 [API_REFERENCE.md](API_REFERENCE.md) 읽기
  - C++ 함수 시그니처
  - JavaScript Module 객체
  - 메모리 관리 API

**실습**:
```javascript
// 메모리 할당/해제 패턴 연습
const ptr = Module._malloc(1000);
try {
    // 처리...
} finally {
    Module._free(ptr);
}
```

**예상 시간**: 4-5시간

---

### 초보자 마일스톤 체크

2주 후 다음을 달성했다면 성공! 🎉

- [ ] 개발 환경 완전 설정
- [ ] 기본 빌드 → 테스트 워크플로우 숙지
- [ ] 세피아 필터 성공적으로 구현
- [ ] C++ ↔ JavaScript 데이터 전달 이해
- [ ] 메모리 할당/해제 패턴 숙지

**다음 단계**: 중급자 경로로 진행

---

## 중급자 경로 (2-4주)

> "기본을 마스터하고 더 복잡한 필터 구현"

### Week 3: 고급 필터 구현

#### Day 15-17: 튜토리얼 2 - 밝기 조절
- [ ] 🎓 [TUTORIAL.md - 밝기 조절 필터](TUTORIAL.md#튜토리얼-2-밝기-조절-필터)
  - 매개변수 전달 방법
  - UI 슬라이더 연동
  - 실시간 값 변경

**학습 목표**:
```cpp
// C++ 매개변수 전달
void applyBrightnessFilterRaw(uintptr_t ptr, int length, int brightness);
//                                                        ↑ 추가 매개변수
```

**실습**:
```
✅ 밝기 조절 슬라이더 구현
✅ 실시간 밝기 변경 테스트
✅ 값 범위 제한 (-100 ~ 100)
```

**예상 시간**: 5-6시간

---

#### Day 18-21: 성능 최적화
- [ ] 📖 [ARCHITECTURE.md - 성능 최적화](ARCHITECTURE.md#성능-최적화-포인트)
  - 메모리 재사용 패턴
  - Canvas 최적화
  - FPS 모니터링

**실습**:
```javascript
// 메모리 재사용 구현
class FilterProcessor {
    constructor(width, height) {
        this.ptr = Module._malloc(width * height * 4);
    }

    applyFilter(imageData, filterFunc) {
        // 메모리 재사용
        Module.HEAPU8.set(imageData.data, this.ptr);
        filterFunc(this.ptr, imageData.data.length);
        imageData.data.set(
            Module.HEAPU8.subarray(this.ptr, this.ptr + imageData.data.length)
        );
    }

    destroy() {
        Module._free(this.ptr);
    }
}
```

**성능 목표**:
- 1280×720 해상도에서 60fps 달성
- 메모리 사용량 100MB 이하 유지

**예상 시간**: 6-8시간

---

### Week 4: 복잡한 알고리즘 구현

#### Day 22-25: 튜토리얼 3 - 블러 효과
- [ ] 🎓 [TUTORIAL.md - 블러 효과](TUTORIAL.md#튜토리얼-3-블러-효과-도전-과제)
  - 2D 이미지 처리
  - 주변 픽셀 참조
  - 임시 버퍼 관리

**학습 목표**:
```cpp
// 3x3 박스 블러 구현
for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
        // 주변 9픽셀 평균 계산
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                // 경계 체크 및 평균 계산
            }
        }
    }
}
```

**도전 과제**:
1. 기본 박스 블러 구현
2. 블러 강도 조절 (3×3, 5×5, 7×7)
3. 가우시안 블러로 업그레이드

**예상 시간**: 8-10시간

---

#### Day 26-28: 독자적인 필터 개발
- [ ] 자유 주제 필터 구현
  - 컬러 반전
  - 채도 조절
  - 샤프닝 효과
  - 엣지 감지

**권장 순서**:
1. 알고리즘 이론 학습 (Wikipedia, OpenCV 문서)
2. 의사 코드(pseudocode) 작성
3. C++ 구현
4. 테스트 및 최적화

**예상 시간**: 10-12시간

---

### 중급자 마일스톤 체크

4주 후 다음을 달성했다면 성공! 🎉

- [ ] 매개변수 전달 방법 이해
- [ ] 성능 최적화 기법 적용
- [ ] 블러 등 복잡한 알고리즘 구현
- [ ] 독자적인 필터 3개 이상 개발
- [ ] 60fps 성능 목표 달성

**다음 단계**: 고급자 경로로 진행

---

## 고급자 경로 (4주+)

> "전문가 수준의 이미지 처리와 최적화"

### 고급 주제

#### 1. SIMD 최적화 (1-2주)
WebAssembly SIMD를 활용한 병렬 처리

**학습 목표**:
```cpp
#include <wasm_simd128.h>

void applyGrayscaleSIMD(uint8_t* data, int length) {
    // 한 번에 16바이트 처리
    for (int i = 0; i < length; i += 16) {
        v128_t pixels = wasm_v128_load(&data[i]);
        // SIMD 연산...
    }
}
```

**성능 목표**: 2-4배 속도 향상

**참고 자료**:
- [WebAssembly SIMD](https://v8.dev/features/simd)
- [Emscripten SIMD Guide](https://emscripten.org/docs/porting/simd.html)

---

#### 2. 멀티스레딩 (2-3주)
Web Workers를 활용한 병렬 처리

**아키텍처**:
```
Main Thread (UI)
    ↓
Worker 1: 상단 절반 처리
Worker 2: 하단 절반 처리
    ↓
결과 병합 → Canvas
```

**학습 목표**:
```javascript
// Web Worker 생성
const worker = new Worker('filter-worker.js');

worker.postMessage({
    imageData: data,
    filter: 'grayscale'
});

worker.onmessage = (e) => {
    const result = e.data;
    // Canvas에 표시
};
```

**성능 목표**: 듀얼코어 이상에서 1.5-2배 속도 향상

---

#### 3. OpenCV.js 통합 (2-3주)
강력한 컴퓨터 비전 라이브러리 활용

**기능 예시**:
- 얼굴 감지 (Haar Cascade)
- 객체 추적
- 특징점 검출 (SIFT, ORB)
- 이미지 세그멘테이션

**참고 자료**:
- [OpenCV.js Tutorial](https://docs.opencv.org/4.x/d5/d10/tutorial_js_root.html)
- [OpenCV.js GitHub](https://github.com/opencv/opencv)

---

#### 4. 실시간 AR 효과 (3-4주)
증강 현실 필터 구현

**프로젝트 아이디어**:
1. 가상 선글라스
2. 얼굴 필터 (고양이 귀, 왕관 등)
3. 배경 교체 (그린스크린)
4. 페이스 트래킹

**기술 스택**:
- MediaPipe (얼굴 랜드마크 감지)
- Three.js (3D 렌더링)
- WebAssembly (고성능 처리)

---

### 전문가 프로젝트

#### 프로젝트 1: 실시간 배경 제거
**난이도**: ⭐⭐⭐⭐
**기술**: 이미지 세그멘테이션, 마스킹

#### 프로젝트 2: 스타일 트랜스퍼
**난이도**: ⭐⭐⭐⭐⭐
**기술**: 딥러닝 모델 (TensorFlow.js), WebAssembly

#### 프로젝트 3: 실시간 포토 부스
**난이도**: ⭐⭐⭐
**기술**: 여러 필터 조합, 이미지 저장, 공유 기능

---

## 실습 프로젝트 아이디어

### 초급 프로젝트

#### 1. 인스타그램 스타일 필터 팩
**필터 종류**:
- 빈티지 (세피아)
- 냉색 톤
- 난색 톤
- 고대비

**기술 포인트**: RGB 색상 변환, 곡선 조정

---

#### 2. 픽셀 아트 생성기
**기능**:
- 이미지를 픽셀 단위로 단순화
- 색상 팔레트 제한
- 크기 조절

**기술 포인트**: 블록 단위 평균, 색상 양자화

---

### 중급 프로젝트

#### 3. 실시간 크로마 키
**기능**:
- 특정 색상(녹색, 파란색) 제거
- 배경 이미지 합성
- 색상 범위 조절

**기술 포인트**: HSV 색공간, 마스킹, 합성

---

#### 4. 포토 에디터
**기능**:
- 밝기, 대비, 채도 조절
- 회전, 크롭
- 필터 레이어링
- 이미지 저장

**기술 포인트**: Canvas 조작, 변환 행렬, Blob API

---

### 고급 프로젝트

#### 5. 실시간 얼굴 필터 앱
**기능**:
- 얼굴 감지 (MediaPipe)
- AR 효과 (선글라스, 모자)
- 표정 인식
- 녹화 기능

**기술 포인트**: 얼굴 랜드마크, 3D 렌더링, WebRTC

---

#### 6. AI 스타일 트랜스퍼
**기능**:
- 사진을 그림 스타일로 변환
- 여러 예술 스타일 선택
- 강도 조절

**기술 포인트**: TensorFlow.js, 사전 학습 모델, GPU 가속

---

## 추천 학습 자료

### 온라인 강좌

#### C++ 기초
- [Learn C++](https://www.learncpp.com/) - 무료, 체계적
- [C++ Primer](https://www.amazon.com/Primer-5th-Stanley-B-Lippman/dp/0321714113) - 책

#### WebAssembly
- [WebAssembly MDN](https://developer.mozilla.org/en-US/docs/WebAssembly)
- [Emscripten Tutorial](https://emscripten.org/docs/getting_started/Tutorial.html)

#### 이미지 처리
- [OpenCV Tutorials](https://docs.opencv.org/4.x/d9/df8/tutorial_root.html)
- [Digital Image Processing (Coursera)](https://www.coursera.org/courses?query=image%20processing)

---

### YouTube 채널
- **The Coding Train**: 창의적인 코딩 튜토리얼
- **Computerphile**: 컴퓨터 과학 개념 설명
- **Two Minute Papers**: AI/CV 최신 연구 소개

---

### 책
1. **"Computer Vision: Algorithms and Applications"** - Richard Szeliski
2. **"Digital Image Processing"** - Rafael C. Gonzalez
3. **"WebAssembly: The Definitive Guide"** - Brian Sletten

---

### 커뮤니티
- [WebAssembly Discord](https://discord.gg/webassembly)
- [r/webassembly](https://www.reddit.com/r/WebAssembly/)
- [Stack Overflow](https://stackoverflow.com/questions/tagged/webassembly)

---

## 학습 팁

### 1. 실습 위주로 학습
```
이론 20% : 실습 80%
```
개념을 읽은 후 반드시 직접 코드를 작성해보세요.

### 2. 작은 목표 설정
```
❌ "웹 앱 완성하기" (너무 큼)
✅ "세피아 필터 만들기" (명확함)
```

### 3. 에러를 두려워하지 말기
에러는 학습의 기회입니다. 에러 메시지를 읽고 이해하세요.

### 4. 코드 리뷰
- GitHub에 코드 공유
- 다른 사람의 코드 읽기
- 피드백 주고받기

### 5. 프로젝트 포트폴리오
완성한 필터를 GitHub Pages로 배포하여 포트폴리오 구축

---

## 다음 단계 결정 가이드

### 현재 레벨 확인

#### 초보자라면:
- [ ] 환경 설정 완료
- [ ] 기본 필터 1개 이상 구현
→ **추천**: Week 1-2 집중

#### 중급자라면:
- [ ] 복잡한 알고리즘 구현 가능
- [ ] 성능 최적화 경험
→ **추천**: Week 3-4, 독자 프로젝트 시작

#### 고급자라면:
- [ ] SIMD, 멀티스레딩 관심
- [ ] OpenCV 등 외부 라이브러리 활용
→ **추천**: 전문가 프로젝트, 오픈소스 기여

---

## 학습 완료 후

### 다음 학습 방향

1. **컴퓨터 비전 심화**
   - OpenCV 마스터
   - 머신러닝 이미지 분류

2. **웹 기술 확장**
   - WebGL (GPU 가속)
   - WebGPU (차세대 그래픽 API)

3. **실전 프로젝트**
   - 오픈소스 기여
   - 개인 서비스 런칭

---

## 성공의 기준

다음 질문에 "예"라고 답할 수 있다면 성공! 🎉

- [ ] 새로운 필터를 처음부터 끝까지 독립적으로 만들 수 있나요?
- [ ] WebAssembly가 왜 필요한지 설명할 수 있나요?
- [ ] 성능 문제를 진단하고 최적화할 수 있나요?
- [ ] C++ ↔ JavaScript 인터페이스를 자유롭게 다룰 수 있나요?

모두 "예"라면 축하합니다! 이제 WebAssembly 이미지 처리 전문가입니다.

---

## 맺음말

이 로드맵은 가이드일 뿐입니다. 자신의 속도에 맞춰 학습하세요.

**핵심은**:
- 꾸준히 실습
- 에러를 즐기기
- 프로젝트로 학습

**화이팅! 🚀**
