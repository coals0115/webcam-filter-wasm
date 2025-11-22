# 🤖 GitHub Actions 완전 정복 가이드

> **대상**: GitHub Actions를 처음 사용하는 2년차 개발자
> **목표**: 따라 치기가 아닌, 완전히 이해하고 응용하기
> **기준**: 이 프로젝트의 실제 `deploy.yml` 파일 분석

---

## 📋 목차

1. [GitHub Actions란?](#github-actions란)
2. [핵심 개념 5가지](#핵심-개념-5가지)
3. [deploy.yml 한 줄씩 완전 분석](#deployyml-한-줄씩-완전-분석)
4. [실전 디버깅 가이드](#실전-디버깅-가이드)
5. [실무 응용 예제](#실무-응용-예제)
6. [자주 묻는 질문](#자주-묻는-질문)

---

## GitHub Actions란?

### 🎯 한 줄 정의

**GitHub에서 제공하는 CI/CD 자동화 플랫폼**

### 🤔 왜 필요한가?

#### 수동 배포의 문제점

```bash
# 개발자가 매번 해야 하는 작업
git pull
./build.sh
git checkout gh-pages
cp -r build/* .
git add .
git commit -m "Deploy"
git push
git checkout main

# 문제:
# - 귀찮음
# - 실수 가능성 (빌드 안 하고 배포, 브랜치 잘못 선택 등)
# - 팀원마다 배포 방식이 다를 수 있음
# - 빌드 환경이 달라서 "내 컴퓨터에선 되는데..." 문제
```

#### GitHub Actions로 해결

```yaml
# main 브랜치에 푸시하면 자동으로:
on:
  push:
    branches: [ main ]

# 1. 코드 받아오기
# 2. 빌드하기
# 3. 배포하기
# → 모든 과정 자동화!
```

**장점**:
- ✅ 일관성: 항상 같은 환경, 같은 방식으로 빌드/배포
- ✅ 신뢰성: 사람 실수 제거
- ✅ 효율성: 개발자는 코드만 작성, 나머지는 자동
- ✅ 투명성: 모든 빌드/배포 과정이 로그로 남음

### 🏗️ GitHub Actions vs 다른 CI/CD

| 도구 | 특징 | 비용 | 설정 난이도 |
|------|------|------|------------|
| **GitHub Actions** | GitHub 통합, YAML 설정 | 무료 (2000분/월) | ⭐⭐ 중간 |
| Jenkins | 자체 호스팅, 유연성 최고 | 서버 비용 | ⭐⭐⭐⭐ 어려움 |
| CircleCI | 빠른 빌드, 병렬 처리 | 유료 (제한적 무료) | ⭐⭐⭐ 중상 |
| Travis CI | 오픈소스 친화적 | 유료 (오픈소스 무료) | ⭐⭐ 중간 |

**GitHub Actions 선택 이유**:
- 이미 GitHub 사용 중 → 별도 설정 불필요
- Private 저장소도 무료 (2000분/월)
- Marketplace에 수천 개의 재사용 가능한 Action
- 이 프로젝트는 5분도 안 걸림 (무료 범위 내)

---

## 핵심 개념 5가지

### 1. Workflow (워크플로우)

**정의**: 자동화할 작업의 전체 흐름

**위치**: `.github/workflows/*.yml`

**예시**:
```
.github/
└── workflows/
    ├── deploy.yml      ← 배포 워크플로우
    ├── test.yml        ← 테스트 워크플로우
    └── lint.yml        ← 코드 검사 워크플로우
```

**비유**: 레시피 전체
- "케이크 만들기" 레시피 = Workflow

### 2. Event (이벤트)

**정의**: 워크플로우를 실행시키는 트리거

**종류**:

```yaml
# 코드 푸시될 때
on:
  push:
    branches: [ main ]

# Pull Request 생성될 때
on:
  pull_request:
    branches: [ main ]

# 매일 자정에 실행
on:
  schedule:
    - cron: '0 0 * * *'

# 수동 실행
on:
  workflow_dispatch:

# 여러 이벤트 조합
on:
  push:
    branches: [ main ]
  pull_request:
    branches: [ main ]
  workflow_dispatch:
```

**비유**: 요리 시작 신호
- "손님이 오면" → 요리 시작 = Event

### 3. Job (작업)

**정의**: 워크플로우 내에서 독립적으로 실행되는 작업 단위

**특징**:
- 여러 Job은 **기본적으로 병렬 실행**
- 필요하면 순차 실행 가능 (`needs` 키워드)
- 각 Job은 독립된 가상 환경에서 실행

**예시**:

```yaml
jobs:
  build:              # Job 1: 빌드
    runs-on: ubuntu-latest
    steps:
      - run: npm run build

  test:               # Job 2: 테스트 (build와 병렬 실행)
    runs-on: ubuntu-latest
    steps:
      - run: npm test

  deploy:             # Job 3: 배포 (build 완료 후 실행)
    needs: build      # ← build Job이 성공해야 실행
    runs-on: ubuntu-latest
    steps:
      - run: npm run deploy
```

**비유**: 요리의 각 단계
- "재료 준비", "조리", "플레이팅" = Jobs

### 4. Step (단계)

**정의**: Job 내에서 순차적으로 실행되는 개별 명령

**특징**:
- 항상 **순차 실행** (위에서 아래로)
- 하나라도 실패하면 다음 Step은 실행 안 됨 (기본값)

**두 가지 타입**:

#### Type A: `run` (직접 명령 실행)

```yaml
steps:
  - name: 파일 목록 보기
    run: ls -la

  - name: 여러 명령 실행
    run: |
      echo "Hello"
      pwd
      npm install
```

#### Type B: `uses` (재사용 가능한 Action 사용)

```yaml
steps:
  - name: 코드 체크아웃
    uses: actions/checkout@v3  # ← 미리 만들어진 Action 사용

  - name: Node.js 설치
    uses: actions/setup-node@v3
    with:
      node-version: '18'
```

**비유**: 레시피의 각 문장
- "물 끓이기", "면 넣기", "3분 기다리기" = Steps

### 5. Runner (러너)

**정의**: 워크플로우를 실제로 실행하는 **가상 서버**

**종류**:

```yaml
runs-on: ubuntu-latest    # Ubuntu Linux (가장 흔함)
runs-on: macos-latest     # macOS
runs-on: windows-latest   # Windows
```

**스펙** (GitHub 제공):
- CPU: 2-core
- RAM: 7 GB
- Storage: 14 GB SSD

**비유**: 주방
- 요리를 실제로 하는 공간 = Runner

---

## deploy.yml 한 줄씩 완전 분석

### 전체 구조 한눈에 보기

```yaml
name: 워크플로우 이름
on: 언제 실행할지 (이벤트)
jobs:
  작업이름:
    runs-on: 어떤 서버에서 실행할지
    steps:
      - 단계1
      - 단계2
      - 단계3
```

---

### 📝 Line 1-2: Workflow 이름

```yaml
name: Deploy to Github Pages
```

**역할**: GitHub Actions 탭에서 보이는 이름

**위치**:
```
GitHub → Actions 탭
└── 워크플로우 목록
    └── "Deploy to Github Pages"  ← 여기 표시됨
```

**팁**: 팀원이 이해하기 쉬운 이름 사용
- ✅ "Deploy to Production"
- ✅ "Run Tests"
- ❌ "Workflow 1"

---

### 📝 Line 3-6: 실행 조건 (Event)

```yaml
on:
  push:
    branches: [ main ]
  workflow_dispatch:
```

#### `push` 이벤트 상세 분석

```yaml
push:
  branches: [ main ]
```

**의미**: "`main` 브랜치에 푸시될 때 실행"

**정확한 트리거 시점**:
```bash
# 로컬에서
git checkout main
git add .
git commit -m "Update code"
git push origin main  # ← 이 순간 워크플로우 실행!
```

**다른 브랜치에 푸시하면?**
```bash
git checkout feature/new-filter
git push origin feature/new-filter  # ← 실행 안 됨
```

**왜 main만?**
- `main` = 배포용 브랜치
- 개발 중인 코드는 배포하면 안 됨
- PR 머지 → main 업데이트 → 자동 배포

#### `workflow_dispatch` 이벤트 상세 분석

```yaml
workflow_dispatch:
```

**의미**: "수동으로도 실행 가능"

**사용 방법**:
1. GitHub → Actions 탭
2. "Deploy to Github Pages" 워크플로우 선택
3. "Run workflow" 버튼 클릭
4. 브랜치 선택 → "Run workflow" 확인

**왜 필요한가?**
- 급하게 재배포 필요할 때
- 테스트용 배포
- 코드 변경 없이 배포만 다시 하고 싶을 때

**실전 예시**:
```
상황: GitHub Pages가 일시적으로 다운됐다가 복구됨
→ 코드 변경 없이 재배포 필요
→ workflow_dispatch로 수동 실행
```

---

### 📝 Line 8-10: Job 정의

```yaml
jobs:
  build-and-deploy:
    runs-on: ubuntu-latest
```

#### Job 이름: `build-and-deploy`

**역할**: 이 Job의 식별자

**어디서 보이나?**
```
GitHub → Actions → 워크플로우 실행 로그
└── build-and-deploy  ← Job 이름으로 표시
    ├── Checkout repository
    ├── Setup Emscripten
    └── ...
```

**네이밍 규칙**:
- 소문자, 하이픈(-), 언더스코어(_) 사용
- 공백 불가
- ✅ `build-and-deploy`
- ✅ `run_tests`
- ❌ `Build And Deploy`

#### Runner: `ubuntu-latest`

```yaml
runs-on: ubuntu-latest
```

**의미**: "최신 Ubuntu Linux 서버에서 실행"

**왜 Ubuntu?**
- Linux 명령어 호환성 (bash, chmod, etc.)
- Emscripten이 Linux에서 가장 잘 작동
- 빠르고 안정적

**선택 가능한 Runner**:
```yaml
runs-on: ubuntu-latest   # Ubuntu 22.04 (현재)
runs-on: ubuntu-20.04    # 특정 버전 지정
runs-on: macos-latest    # macOS (iOS 빌드 등)
runs-on: windows-latest  # Windows (Windows 앱 빌드 등)
```

**비용**:
```
Linux   (ubuntu): 1배 소비
macOS   (macos):  10배 소비  ← 비쌈!
Windows (windows): 2배 소비
```

---

### 📝 Line 12-17: Step 1 - 코드 체크아웃

```yaml
steps:
# 1. 코드 체크아웃
- name: Checkout repository
  uses: actions/checkout@v3
  with:
    submodules: true # emsdk 서브모듈 포함
```

#### `name`: Step 이름

```yaml
name: Checkout repository
```

**역할**: 로그에서 보이는 이름 (선택사항이지만 강력 권장)

**로그 예시**:
```
✅ Checkout repository
   - Fetching the repository
   - Checking out ref
   - Complete
```

#### `uses`: 미리 만들어진 Action 사용

```yaml
uses: actions/checkout@v3
```

**의미**: "GitHub 공식 `checkout` Action의 버전 3 사용"

**`actions/checkout`이 하는 일**:
```bash
# 실제로는 이렇게 복잡한 작업을 자동으로 해줌
git clone https://github.com/coals0115/webcam-filter-wasm.git
cd webcam-filter-wasm
git checkout main
git submodule update --init --recursive
```

**왜 필요한가?**
- Runner(가상 서버)는 빈 상태로 시작
- 코드가 없으면 빌드할 수 없음
- `checkout` Action이 코드를 Runner로 복사

**버전 관리 (@v3)**:
```yaml
uses: actions/checkout@v3   # 메이저 버전 3 (권장)
uses: actions/checkout@v3.5.2  # 정확한 버전
uses: actions/checkout@main    # 최신 개발 버전 (불안정)
```

**팁**: 항상 메이저 버전(`@v3`) 사용 권장
- 보안 패치 자동 적용
- 호환성 유지

#### `with`: Action에 옵션 전달

```yaml
with:
  submodules: true
```

**의미**: "Git 서브모듈도 함께 체크아웃"

**서브모듈이란?**
```
webcam-filter-wasm/
└── emsdk/  ← Git 서브모듈 (다른 저장소)
```

**왜 필요한가?**
- 이 프로젝트는 `emsdk`를 서브모듈로 포함
- `submodules: true` 없으면 빈 폴더로 남음
- Emscripten 빌드 실패

**확인 방법**:
```bash
# 프로젝트 루트에서
ls -la .gitmodules  # 서브모듈 설정 파일 확인
```

---

### 📝 Line 19-23: Step 2 - Emscripten 설치

```yaml
# 2. Emscripten 설치
- name: Setup Emscripten
  uses: mymindstorm/setup-emsdk@v12
  with:
    version: 'latest'
```

#### `uses`: 커뮤니티 Action 사용

```yaml
uses: mymindstorm/setup-emsdk@v12
```

**의미**: "누군가 만들어둔 Emscripten 설치 Action 사용"

**어디서 찾았나?**
- [GitHub Marketplace](https://github.com/marketplace)에서 검색
- "emscripten" 검색 → `setup-emsdk` 발견

**이 Action이 하는 일**:
```bash
# 자동으로 실행되는 명령들
curl -o emsdk-installer.sh https://...
chmod +x emsdk-installer.sh
./emsdk-installer.sh
emsdk install latest
emsdk activate latest
source emsdk_env.sh
```

**만약 이 Action이 없었다면?**
```yaml
# 직접 작성해야 함 (복잡!)
- name: Install Emscripten manually
  run: |
    git clone https://github.com/emscripten-core/emsdk.git
    cd emsdk
    ./emsdk install latest
    ./emsdk activate latest
    source ./emsdk_env.sh
    echo "EMSDK=$PWD" >> $GITHUB_ENV
    # ... 20줄 이상의 설정
```

**교훈**: 이미 만들어진 Action을 적극 활용하자!

#### `with.version`: Emscripten 버전 지정

```yaml
with:
  version: 'latest'
```

**선택지**:
```yaml
version: 'latest'      # 최신 버전 (권장)
version: '3.1.45'      # 특정 버전 고정
```

**버전 고정 vs 최신**:

| 방식 | 장점 | 단점 |
|------|------|------|
| `latest` | 최신 기능, 버그 수정 | 호환성 문제 가능 |
| `3.1.45` | 재현 가능, 안정적 | 보안 패치 누락 |

**이 프로젝트는 `latest` 선택 이유**:
- 간단한 프로젝트라 호환성 문제 적음
- 최신 최적화 기능 활용

---

### 📝 Line 25-29: Step 3 - 프로젝트 빌드

```yaml
# 3. 프로젝트 빌드
- name: Build WASM
  run: |
    chmod +x build.sh
    ./build.sh
```

#### `run`: 직접 명령 실행

```yaml
run: |
  chmod +x build.sh
  ./build.sh
```

**`|` (파이프) 의미**: "여러 줄 명령어"

**한 줄 vs 여러 줄**:

```yaml
# 한 줄
run: echo "Hello"

# 여러 줄
run: |
  echo "Hello"
  echo "World"
  pwd
```

#### `chmod +x build.sh`: 실행 권한 부여

**왜 필요한가?**

**문제**:
```bash
./build.sh
# bash: ./build.sh: Permission denied  ← 실행 권한 없음
```

**해결**:
```bash
chmod +x build.sh  # 실행 권한 부여
./build.sh         # 이제 실행 가능
```

**왜 매번 필요한가?**
- Git은 실행 권한을 저장하지만, 때때로 누락됨
- Runner 환경에서는 안전하게 항상 실행

**대안**:
```yaml
# 직접 bash로 실행 (권한 불필요)
run: bash build.sh
```

#### `./build.sh`: 빌드 스크립트 실행

**이 스크립트가 하는 일**:
```bash
# build.sh 내용 (요약)
emcc src/main.cpp -o build/filters.js \
  -s WASM=1 \
  -O3 \
  --bind
```

**결과물**:
```
build/
├── filters.wasm  ← C++를 컴파일한 WebAssembly
├── filters.js    ← WASM 로더
└── index.html    ← 웹 페이지
```

---

### 📝 Line 31-33: Step 4 - Jekyll 비활성화

```yaml
# 4. Jekyll 비활성화
- name: Disable Jekyll
  run: touch build/.nojekyll
```

#### `touch build/.nojekyll`: 빈 파일 생성

**명령어 분석**:
```bash
touch build/.nojekyll
```

**`touch` 명령어**:
- 파일이 없으면 생성
- 파일이 있으면 수정 시간만 업데이트
- 내용이 없는 빈 파일 생성

**결과**:
```bash
ls -la build/.nojekyll
# -rw-r--r--  1 runner  staff  0B  Nov 22 12:00 .nojekyll
#                               ↑ 0바이트 (빈 파일)
```

#### Jekyll이란? (심화 학습)

**Jekyll = 정적 사이트 생성기 (Static Site Generator)**

Markdown 파일을 HTML 웹사이트로 자동 변환해주는 도구입니다.

**Jekyll의 작동 방식**:

```
입력: Markdown 파일
---
layout: post
title: "블로그 글"
---

# 제목
본문입니다.

  ↓ Jekyll 변환

출력: HTML 파일
<!DOCTYPE html>
<html>
  <head><title>블로그 글</title></head>
  <body>
    <h1>제목</h1>
    <p>본문입니다.</p>
  </body>
</html>
```

**Jekyll이 유용한 경우**:
```
블로그 프로젝트/
├── _posts/
│   ├── 2025-11-22-hello.md     ← Markdown으로 글 작성
│   └── 2025-11-23-world.md
├── _config.yml                  ← Jekyll 설정
└── index.md

→ Jekyll 변환 후 자동으로 예쁜 블로그 사이트 생성!
```

**GitHub Pages와 Jekyll**:

GitHub Pages는 **Jekyll을 기본으로 내장**하고 있습니다.

```
파일 업로드
  ↓
GitHub Pages: "Markdown 파일이 있네?"
  ↓
Jekyll 자동 실행
  ↓
HTML 변환
  ↓
웹사이트 배포
```

#### 왜 `.nojekyll`이 필요한가?

**우리 프로젝트 구조**:
```
build/
├── index.html       ← 이미 완성된 HTML
├── app.js           ← JavaScript
├── filters.wasm     ← WebAssembly
└── styles.css       ← CSS
```

**특징**: 이미 완성된 정적 파일 (변환 불필요)

**Jekyll이 활성화되어 있으면?**:
```
GitHub Pages: "파일들을 Jekyll로 처리해야겠군!"
  ↓
Jekyll 실행 시도
  ↓
문제 발생:
- .wasm 파일을 잘못 처리할 수 있음
- _로 시작하는 파일 무시 (Jekyll 규칙)
- 불필요한 변환 시도로 배포 지연
```

**`.nojekyll` 파일의 역할**:
```
GitHub Pages: ".nojekyll 파일이 있네?"
  ↓
"아, 이미 완성된 HTML이구나"
  ↓
Jekyll 건너뛰기
  ↓
파일 그대로 서빙 (빠르고 안전)
```

**Jekyll 비활성화 결과**:
- ✅ WASM 파일 안전하게 로드
- ✅ 빠른 배포 (변환 과정 없음)
- ✅ 파일 변조 없음

**언제 Jekyll을 비활성화해야 하나?**

| 프로젝트 타입 | Jekyll 사용 | `.nojekyll` 필요 |
|--------------|------------|-----------------|
| 블로그 (Markdown) | ✅ 사용 | ❌ 불필요 |
| 문서 사이트 (Markdown) | ✅ 사용 | ❌ 불필요 |
| React/Vue 앱 | ❌ 불필요 | ✅ 필요 |
| WASM 프로젝트 (우리) | ❌ 불필요 | ✅ 필요 |
| 순수 HTML/CSS/JS | ❌ 불필요 | ✅ 필요 |

**GitHub Pages Jekyll 감지 규칙**:
```
1. .nojekyll 파일 있음?
   └─ YES → Jekyll 비활성화 ✅
   └─ NO  → 2번으로

2. _config.yml 파일 있음?
   └─ YES → Jekyll 활성화
   └─ NO  → 3번으로

3. .md 파일 있음?
   └─ YES → Jekyll 활성화
   └─ NO  → Jekyll 비활성화
```

**실전 팁**:
```bash
# 배포 전 항상 .nojekyll 파일 생성 확인
ls -la build/.nojekyll

# 없으면 생성
touch build/.nojekyll

# GitHub Actions에서는 자동으로 생성됨
- name: Disable Jekyll
  run: touch build/.nojekyll
```

---

### 📝 Line 35-41: Step 5 - GitHub Pages 배포

```yaml
# 5. Github Pages에 배포
- name: Deploy to Github Pages
  uses: peaceiris/actions-gh-pages@v3
  with:
    github_token: ${{ secrets.GITHUB_TOKEN }}
    publish_dir: ./build
    publish_branch: gh-pages
```

#### `uses`: 배포 전문 Action 사용

```yaml
uses: peaceiris/actions-gh-pages@v3
```

**peaceiris/actions-gh-pages**:
- GitHub Pages 배포 전문 Action
- ⭐ 10,000+ stars (인기 많음)
- 자동으로 gh-pages 브랜치 생성 및 푸시

**이 Action이 없었다면?**
```yaml
# 직접 작성해야 할 코드 (30줄 이상)
- run: |
    git config user.name github-actions
    git config user.email github-actions@github.com
    git checkout -b gh-pages
    cp -r build/* .
    git add .
    git commit -m "Deploy"
    git push origin gh-pages
    # ... 에러 처리, 브랜치 존재 여부 확인 등
```

#### `github_token`: 인증 토큰

```yaml
github_token: ${{ secrets.GITHUB_TOKEN }}
```

**`${{ }}` 문법**: 표현식 (Expression)

**의미**: "변수 값을 여기에 삽입"

**`secrets.GITHUB_TOKEN`**:
- GitHub Actions가 **자동으로 생성**하는 토큰
- 별도 설정 불필요
- 워크플로우 실행 시 자동 주입

**왜 필요한가?**
- `gh-pages` 브랜치에 푸시하려면 권한 필요
- 토큰 = 신분증

**보안**:
```yaml
github_token: ${{ secrets.GITHUB_TOKEN }}
# 로그에 표시될 때:
# github_token: ***  ← 자동으로 숨겨짐
```

**권한 범위**:
- Repository 읽기/쓰기
- 워크플로우 종료 시 자동 만료

#### `publish_dir`: 배포할 폴더

```yaml
publish_dir: ./build
```

**의미**: "`./build` 폴더의 내용을 배포"

**폴더 구조**:
```
프로젝트 루트/
├── src/           ← 소스 코드 (배포 안 함)
├── docs/          ← 문서 (배포 안 함)
└── build/         ← 빌드 결과물 (배포함!)
    ├── index.html
    ├── filters.wasm
    └── ...
```

**결과**:
```
gh-pages 브랜치에는 build/ 내용만 복사됨
├── index.html
├── filters.wasm
└── ...
```

**왜 build/만?**
- 소스 코드 노출 방지
- 깔끔한 URL: `https://...io/webcam-filter-wasm/` (build/ 경로 없음)

#### `publish_branch`: 배포 브랜치

```yaml
publish_branch: gh-pages
```

**의미**: "`gh-pages` 브랜치에 배포"

**브랜치 역할**:
```
main 브랜치:
- 소스 코드 저장
- 개발 작업

gh-pages 브랜치:
- 빌드 결과물만 저장
- GitHub Pages가 읽어서 웹사이트로 서빙
```

**자동으로 하는 일**:
```bash
# 1. gh-pages 브랜치 존재 확인
# 2. 없으면 생성
git checkout --orphan gh-pages

# 3. build/ 내용 복사
cp -r build/* .

# 4. 커밋 및 푸시
git add .
git commit -m "Deploy"
git push origin gh-pages
```

---

## 실전 디버깅 가이드

### 🔍 워크플로우가 실행 안 될 때

#### 체크리스트

1. **파일 위치 확인**
```bash
ls -la .github/workflows/deploy.yml
# 정확히 이 경로에 있어야 함
```

2. **YAML 문법 오류 확인**
- 들여쓰기 2칸 (탭 사용 금지!)
- 콜론(`:`) 뒤 공백 필수

```yaml
# ❌ 잘못된 예
name:Deploy      # 공백 없음
  steps:         # 들여쓰기 이상함
- name:Test      # 공백 없음

# ✅ 올바른 예
name: Deploy     # 공백 있음
  steps:         # 정확한 들여쓰기
    - name: Test # 공백 있음
```

3. **브랜치 확인**
```yaml
on:
  push:
    branches: [ main ]  # ← main 브랜치에만 반응
```

```bash
# 현재 브랜치 확인
git branch
# * feat/something  ← main이 아니면 실행 안 됨
```

### 🐛 빌드 실패 디버깅

#### Actions 로그 읽는 법

**로그 위치**:
```
GitHub → Actions 탭 → 실패한 워크플로우 클릭
```

**로그 구조**:
```
✅ Set up job
✅ Checkout repository
✅ Setup Emscripten
❌ Build WASM          ← 여기서 실패!
   │
   └── Error: command not found: emcc  ← 에러 메시지
```

#### 자주 발생하는 에러

**1. `Permission denied: ./build.sh`**

```
Error: bash: ./build.sh: Permission denied
```

**원인**: 실행 권한 없음

**해결**:
```yaml
- name: Build WASM
  run: |
    chmod +x build.sh  # ← 추가
    ./build.sh
```

**2. `emcc: command not found`**

```
Error: emcc: command not found
```

**원인**: Emscripten 설치 안 됨

**해결**:
```yaml
# Setup Emscripten step이 있는지 확인
- name: Setup Emscripten
  uses: mymindstorm/setup-emsdk@v12
```

**3. `remote: Permission denied to github-actions[bot]`**

```
Error: remote: Permission to repo.git denied to github-actions[bot]
```

**원인**: Workflow 쓰기 권한 없음

**해결**:
```
Settings → Actions → General
→ Workflow permissions
  → ✅ Read and write permissions
```

**4. `No such file or directory: build/`**

```
Error: cp: build/: No such file or directory
```

**원인**: 빌드 스크립트가 `build/` 폴더를 안 만듦

**해결**:
```yaml
- name: Build WASM
  run: |
    mkdir -p build  # ← 폴더 먼저 생성
    ./build.sh
```

### 🧪 로컬에서 테스트하는 법

**문제**: GitHub Actions는 푸시할 때마다 실행 → 느림

**해결**: [act](https://github.com/nektos/act)로 로컬 테스트

#### act 설치 및 사용

```bash
# macOS
brew install act

# 워크플로우 실행
act push

# 특정 Job만 실행
act -j build-and-deploy

# 이벤트 시뮬레이션
act workflow_dispatch
```

**장점**:
- ✅ 푸시 전에 미리 테스트
- ✅ 빠른 피드백
- ✅ 무료 Actions 시간 절약

**제한사항**:
- 완벽하게 동일한 환경은 아님
- 일부 GitHub 전용 기능 동작 안 함

---

## 실무 응용 예제

### 📧 예제 1: 빌드 실패 시 Slack 알림

```yaml
jobs:
  build-and-deploy:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout
        uses: actions/checkout@v3

      - name: Build
        id: build  # ← ID 부여
        run: ./build.sh

      # 빌드 실패 시만 실행
      - name: Notify Slack on failure
        if: failure() && steps.build.outcome == 'failure'
        uses: 8398a7/action-slack@v3
        with:
          status: ${{ job.status }}
          text: 'Build failed! Check logs.'
          webhook_url: ${{ secrets.SLACK_WEBHOOK }}
```

**핵심**:
- `id`: Step에 식별자 부여
- `if: failure()`: 실패 시만 실행
- `secrets.SLACK_WEBHOOK`: Repository Secrets에 저장

### 🧪 예제 2: PR에서 자동 테스트

```yaml
name: Run Tests on PR

on:
  pull_request:
    branches: [ main ]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Run tests
        run: npm test

      # 테스트 결과를 PR에 코멘트
      - name: Comment test results
        uses: actions/github-script@v6
        if: always()
        with:
          script: |
            github.rest.issues.createComment({
              issue_number: context.issue.number,
              owner: context.repo.owner,
              repo: context.repo.repo,
              body: '✅ All tests passed!'
            })
```

### 🏷️ 예제 3: 버전 태그 자동 생성

```yaml
name: Auto Tag on Release

on:
  push:
    branches: [ main ]

jobs:
  tag:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3

      - name: Read version from package.json
        id: version
        run: echo "VERSION=$(jq -r .version package.json)" >> $GITHUB_OUTPUT

      - name: Create tag
        run: |
          git config user.name github-actions
          git config user.email github-actions@github.com
          git tag v${{ steps.version.outputs.VERSION }}
          git push origin v${{ steps.version.outputs.VERSION }}
```

### 🎨 예제 4: 여러 환경에서 테스트 (Matrix)

```yaml
jobs:
  test:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        node: [16, 18, 20]

    steps:
      - uses: actions/checkout@v3

      - name: Setup Node.js
        uses: actions/setup-node@v3
        with:
          node-version: ${{ matrix.node }}

      - name: Run tests
        run: npm test
```

**결과**: 3개 OS × 3개 Node 버전 = 9개 Job 동시 실행

### ⏰ 예제 5: 매일 자동 배포

```yaml
name: Daily Deployment

on:
  schedule:
    - cron: '0 0 * * *'  # 매일 자정 (UTC)

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - run: ./build.sh
      - uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./build
```

**Cron 문법**:
```
* * * * *
│ │ │ │ │
│ │ │ │ └─ 요일 (0-6, 0=일요일)
│ │ │ └─── 월 (1-12)
│ │ └───── 일 (1-31)
│ └─────── 시 (0-23)
└───────── 분 (0-59)
```

**예시**:
```yaml
- cron: '0 9 * * 1-5'  # 주중 오전 9시
- cron: '0 0 1 * *'    # 매월 1일 자정
- cron: '0 */6 * * *'  # 6시간마다
```

---

## 자주 묻는 질문

### Q1: Secrets는 어떻게 추가하나요?

**위치**:
```
Repository → Settings → Secrets and variables → Actions
→ "New repository secret" 버튼
```

**추가 방법**:
1. Name: `SLACK_WEBHOOK`
2. Secret: `https://hooks.slack.com/...`
3. "Add secret" 클릭

**사용**:
```yaml
env:
  MY_SECRET: ${{ secrets.SLACK_WEBHOOK }}
```

**주의**: Secrets는 로그에 `***`로 표시됨 (보안)

### Q2: 워크플로우 실행 시간 제한은?

**제한**:
- Job당 최대: 6시간
- Workflow당 최대: 72시간

**무료 플랜**:
- Public 저장소: 무제한
- Private 저장소: 2000분/월

**시간 확인**:
```
Settings → Billing → Plans and usage
→ Actions usage
```

### Q3: 환경 변수는 어떻게 설정하나요?

**방법 1: 워크플로우 레벨**
```yaml
env:
  NODE_ENV: production

jobs:
  build:
    steps:
      - run: echo $NODE_ENV  # production
```

**방법 2: Job 레벨**
```yaml
jobs:
  build:
    env:
      BUILD_TYPE: release
    steps:
      - run: echo $BUILD_TYPE
```

**방법 3: Step 레벨**
```yaml
steps:
  - name: Build
    env:
      OPTIMIZE: true
    run: ./build.sh
```

**동적 환경 변수**:
```yaml
- name: Set version
  run: echo "VERSION=1.2.3" >> $GITHUB_ENV

- name: Use version
  run: echo $VERSION  # 1.2.3
```

### Q4: 다른 워크플로우 결과를 기다릴 수 있나요?

**방법 1: `needs` 사용 (같은 워크플로우 내)**
```yaml
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - run: npm run build

  test:
    needs: build  # build 완료 후 실행
    runs-on: ubuntu-latest
    steps:
      - run: npm test

  deploy:
    needs: [build, test]  # 둘 다 완료 후 실행
    runs-on: ubuntu-latest
    steps:
      - run: npm run deploy
```

**방법 2: `workflow_run` 이벤트 (다른 워크플로우)**
```yaml
# deploy.yml
on:
  workflow_run:
    workflows: ["Run Tests"]  # 이 워크플로우 완료 후
    types:
      - completed
    branches:
      - main
```

### Q5: 캐싱으로 빌드 속도를 높일 수 있나요?

**예시: npm 패키지 캐싱**
```yaml
- name: Cache node modules
  uses: actions/cache@v3
  with:
    path: ~/.npm
    key: ${{ runner.os }}-node-${{ hashFiles('**/package-lock.json') }}
    restore-keys: |
      ${{ runner.os }}-node-

- name: Install dependencies
  run: npm ci
```

**예시: 빌드 결과물 캐싱**
```yaml
- name: Cache build
  uses: actions/cache@v3
  with:
    path: build/
    key: ${{ runner.os }}-build-${{ hashFiles('src/**') }}
```

**효과**:
- 첫 실행: 5분
- 캐시 히트: 30초

### Q6: PR에 자동으로 코멘트할 수 있나요?

```yaml
- name: Comment PR
  uses: actions/github-script@v6
  with:
    script: |
      github.rest.issues.createComment({
        issue_number: context.issue.number,
        owner: context.repo.owner,
        repo: context.repo.repo,
        body: '✅ Build successful! Preview: https://...'
      })
```

### Q7: 조건부 실행은 어떻게 하나요?

**파일 변경 감지**:
```yaml
- name: Deploy docs
  if: contains(github.event.head_commit.modified, 'docs/')
  run: ./deploy-docs.sh
```

**브랜치별 실행**:
```yaml
- name: Deploy to production
  if: github.ref == 'refs/heads/main'
  run: ./deploy-prod.sh

- name: Deploy to staging
  if: github.ref == 'refs/heads/develop'
  run: ./deploy-staging.sh
```

**이전 Step 결과 확인**:
```yaml
- name: Build
  id: build
  run: ./build.sh
  continue-on-error: true  # 실패해도 계속

- name: Notify failure
  if: steps.build.outcome == 'failure'
  run: echo "Build failed!"
```

---

## 🎓 학습 로드맵

### 1주차: 기초 다지기
- [x] 이 문서 정독
- [ ] `deploy.yml` 직접 작성해보기
- [ ] 간단한 수정 (커밋 메시지 변경 등)

### 2주차: 실전 적용
- [ ] 테스트 자동화 추가
- [ ] 환경 변수 사용해보기
- [ ] 실패 시 알림 설정

### 3주차: 고급 기능
- [ ] Matrix 빌드 설정
- [ ] 캐싱 적용
- [ ] 여러 워크플로우 연동

### 학습 자료

**공식 문서**:
- [GitHub Actions 공식 문서](https://docs.github.com/en/actions)
- [Workflow 문법](https://docs.github.com/en/actions/using-workflows/workflow-syntax-for-github-actions)
- [Marketplace](https://github.com/marketplace?type=actions)

**추천 튜토리얼**:
- [GitHub Actions Tutorial - 한국어](https://www.daleseo.com/github-actions-basics/)
- [Learn GitHub Actions](https://docs.github.com/en/actions/learn-github-actions)

**실전 예제 저장소**:
- [actions/starter-workflows](https://github.com/actions/starter-workflows)
- [이 프로젝트의 워크플로우](../.github/workflows/deploy.yml)

---

## 🎯 핵심 요약

### GitHub Actions 3줄 요약
1. **YAML 파일**로 자동화 워크플로우 정의
2. **이벤트 발생** 시 자동 실행 (push, PR, schedule 등)
3. **가상 서버**에서 빌드/테스트/배포 자동화

### 우리 프로젝트 워크플로우 요약
```
main 브랜치 푸시
  ↓
Ubuntu 서버 생성
  ↓
코드 다운로드 (checkout)
  ↓
Emscripten 설치
  ↓
C++ → WASM 빌드
  ↓
gh-pages 브랜치에 배포
  ↓
GitHub Pages 자동 업데이트
  ↓
웹사이트 Live! 🎉
```

### 꼭 기억할 5가지

1. **`.github/workflows/*.yml`**: 워크플로우 파일 위치
2. **`on`**: 언제 실행할지 (push, PR, schedule 등)
3. **`jobs`**: 독립적인 작업 단위 (병렬 실행 가능)
4. **`steps`**: 순차적으로 실행되는 명령
5. **`uses` vs `run`**: 재사용 Action vs 직접 명령

---

## 다음 단계

이제 GitHub Actions의 기초를 완전히 이해했습니다! 🎉

**실습 과제**:
1. 커밋 메시지에 이모지 자동 추가하기
2. 빌드 시간 측정 및 출력하기
3. 배포 완료 시 Slack 알림 보내기

**질문이 있다면**:
- [GitHub Issues](../../issues)에 질문 올리기
- [GitHub Discussions](../../discussions)에서 토론하기

**Happy Automating! 🚀**
