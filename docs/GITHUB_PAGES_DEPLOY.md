# 🚀 GitHub Pages 배포 가이드

> **초보자를 위한 단계별 GitHub Pages 배포 완벽 가이드**
>
> 이 문서는 처음 GitHub Pages를 사용하는 개발자를 위해 작성되었습니다.

---

## 📋 목차

1. [GitHub Pages란?](#github-pages란)
2. [사전 준비사항](#사전-준비사항)
3. [배포 방법](#배포-방법)
   - [방법 1: 수동 배포 (권장 - 가장 쉬움)](#방법-1-수동-배포-권장---가장-쉬움)
   - [방법 2: GitHub Actions 자동 배포](#방법-2-github-actions-자동-배포)
   - [방법 3: Git Subtree 배포](#방법-3-git-subtree-배포)
4. [배포 확인 및 테스트](#배포-확인-및-테스트)
5. [문제 해결](#문제-해결)
6. [배포 후 업데이트 방법](#배포-후-업데이트-방법)

---

## GitHub Pages란?

**GitHub Pages**는 GitHub에서 제공하는 **무료 정적 웹사이트 호스팅 서비스**입니다.

### 특징
- ✅ **완전 무료**: 대역폭 제한 없음
- ✅ **간단한 배포**: Git push만으로 자동 배포
- ✅ **HTTPS 지원**: 보안 인증서 자동 제공
- ✅ **커스텀 도메인 지원**: 원하는 도메인 연결 가능
- ⚠️ **정적 파일만 지원**: HTML, CSS, JS, 이미지 등 (서버 사이드 코드 불가)

### 이 프로젝트에 적합한 이유
우리 프로젝트는 순수 HTML, CSS, JavaScript, WASM으로만 구성되어 있어 **GitHub Pages에 완벽하게 호환**됩니다!

---

## 사전 준비사항

### 1️⃣ GitHub 계정
- GitHub 계정이 없다면 [github.com](https://github.com)에서 가입하세요.

### 2️⃣ GitHub 저장소 생성
- 이미 저장소가 있다면 이 단계는 건너뛰세요.
- 없다면 새 저장소를 만드세요:
  ```bash
  # GitHub 웹사이트에서 New Repository 클릭
  # 또는 터미널에서:
  gh repo create webcam-filter-wasm --public
  ```

### 3️⃣ 프로젝트 빌드 완료
배포 전에 먼저 프로젝트를 빌드해야 합니다:

#### macOS / Linux
```bash
# Emscripten 환경 활성화
cd emsdk && source ./emsdk_env.sh && cd ..

# 프로젝트 빌드
./build.sh
```

#### Windows (PowerShell)
```powershell
# Emscripten 환경 활성화
cd emsdk
.\emsdk_env.ps1
cd ..

# 프로젝트 빌드
.\build.ps1
```

#### Windows (Git Bash)
```bash
# macOS/Linux와 동일
cd emsdk && source ./emsdk_env.sh && cd ..
./build.sh
```

**확인**: `build/` 폴더에 다음 파일들이 있어야 합니다:
- `index.html`
- `app.js`
- `wrapper.js`
- `styles.css`
- `filters.wasm`
- `filters.js`

---

## 배포 방법

### 방법 1: 수동 배포 (권장 - 가장 쉬움)

**난이도**: ⭐ (초급)
**소요 시간**: 5분
**추천 대상**: 처음 GitHub Pages를 사용하는 분

#### 1단계: 빌드 폴더 확인

```bash
# 현재 위치 확인
pwd
# 출력 예: /Users/username/CLionProjects/webcam-filter-wasm

# 빌드 폴더 내용 확인
ls build/
# 다음 파일들이 보여야 합니다:
# index.html  app.js  wrapper.js  styles.css  filters.wasm  filters.js
```

#### 2단계: gh-pages 브랜치 생성

```bash
# 현재 변경사항 저장 (있다면)
git add .
git commit -m "Update before deploy"

# gh-pages 브랜치 생성 및 전환
git checkout -b gh-pages
```

**설명**:
- `gh-pages`는 GitHub Pages 전용 브랜치입니다.
- 이 브랜치의 내용이 웹사이트로 배포됩니다.

#### 3단계: 빌드 파일을 루트로 복사

```bash
# build/ 폴더 내용을 현재 폴더(루트)로 복사
cp -r build/* .

# Jekyll 비활성화 파일 생성 (중요!)
touch .nojekyll
```

**`.nojekyll` 파일이 필요한 이유**:
- GitHub Pages는 기본적으로 Jekyll(정적 사이트 생성기)을 사용합니다.
- `.nojekyll` 파일이 있으면 Jekyll을 건너뛰고 파일을 그대로 서빙합니다.
- WASM 파일이 제대로 로드되려면 필수입니다.

#### 4단계: 커밋 및 푸시

```bash
# 모든 변경사항 스테이징
git add .

# 커밋
git commit -m "Deploy to GitHub Pages"

# GitHub에 푸시
git push origin gh-pages
```

**만약 오류가 발생한다면**:
```bash
# 원격 저장소가 설정되지 않은 경우
git remote add origin https://github.com/USERNAME/REPO_NAME.git

# 다시 푸시
git push origin gh-pages
```

#### 5단계: GitHub 저장소 설정

1. GitHub 웹사이트에서 저장소로 이동
2. **Settings** 탭 클릭
3. 왼쪽 메뉴에서 **Pages** 클릭
4. **Source** 섹션에서:
   - Branch: `gh-pages` 선택
   - Folder: `/ (root)` 선택
   - **Save** 버튼 클릭

![GitHub Pages Settings](https://docs.github.com/assets/cb-47267/mw-1440/images/help/pages/select-branch.webp)

#### 6단계: 배포 완료! 🎉

- 1~2분 후 배포가 완료됩니다.
- 페이지 상단에 배포 URL이 표시됩니다:
  ```
  Your site is live at https://USERNAME.github.io/webcam-filter-wasm/
  ```

#### 7단계: 원래 브랜치로 복귀

```bash
# main 브랜치로 돌아가기
git checkout main
```

---

### 방법 2: GitHub Actions 자동 배포

**난이도**: ⭐⭐ (중급)
**소요 시간**: 10분
**추천 대상**: 코드 변경 시 자동으로 배포하고 싶은 분

#### 1단계: GitHub Actions 워크플로우 파일 생성

프로젝트 루트에 `.github/workflows/deploy.yml` 파일을 생성합니다:

```bash
# 디렉토리 생성
mkdir -p .github/workflows

# 파일 생성 (아래 내용 복사)
touch .github/workflows/deploy.yml
```

#### 2단계: deploy.yml 내용 작성

`.github/workflows/deploy.yml` 파일에 다음 내용을 추가:

```yaml
name: Deploy to GitHub Pages

# main 브랜치에 푸시될 때 자동 실행
on:
  push:
    branches: [ main ]
  workflow_dispatch:  # 수동 실행도 가능

jobs:
  build-and-deploy:
    runs-on: ubuntu-latest

    steps:
    # 1. 코드 체크아웃
    - name: Checkout repository
      uses: actions/checkout@v3
      with:
        submodules: true  # emsdk 서브모듈 포함

    # 2. Emscripten 설치
    - name: Setup Emscripten
      uses: mymindstorm/setup-emsdk@v12
      with:
        version: 'latest'

    # 3. 프로젝트 빌드
    - name: Build WASM
      run: |
        chmod +x build.sh
        ./build.sh

    # 4. Jekyll 비활성화
    - name: Disable Jekyll
      run: touch build/.nojekyll

    # 5. GitHub Pages에 배포
    - name: Deploy to GitHub Pages
      uses: peaceiris/actions-gh-pages@v3
      with:
        github_token: ${{ secrets.GITHUB_TOKEN }}
        publish_dir: ./build
        publish_branch: gh-pages
```

#### 3단계: 파일 커밋 및 푸시

```bash
# 워크플로우 파일 추가
git add .github/workflows/deploy.yml

# 커밋
git commit -m "Add GitHub Actions deploy workflow"

# 푸시
git push origin main
```

#### 4단계: Actions 실행 확인

1. GitHub 저장소로 이동
2. **Actions** 탭 클릭
3. "Deploy to GitHub Pages" 워크플로우가 실행 중인지 확인
4. 완료되면 초록색 체크 표시가 나타남

#### 5단계: GitHub Pages 설정

- Settings → Pages → Source: `gh-pages` 브랜치 선택
- **이제 main 브랜치에 푸시할 때마다 자동으로 배포됩니다!**

---

### 방법 3: Git Subtree 배포

**난이도**: ⭐⭐⭐ (고급)
**소요 시간**: 3분
**추천 대상**: Git에 익숙한 개발자

#### 단계: 한 줄로 배포

```bash
# build 폴더만 gh-pages 브랜치에 푸시
git subtree push --prefix build origin gh-pages
```

**장점**:
- 한 번에 배포 가능
- 빌드 폴더만 깔끔하게 배포됨

**단점**:
- 처음 실행 시 시간이 오래 걸릴 수 있음
- Git 히스토리가 복잡해질 수 있음

**사용 예시**:
```bash
# 1. 프로젝트 빌드
./build.sh

# 2. 빌드 결과 커밋
git add build/
git commit -m "Update build files"

# 3. Subtree로 배포
git subtree push --prefix build origin gh-pages
```

---

## 배포 확인 및 테스트

### 1️⃣ 배포 상태 확인

GitHub 저장소에서:
- **Settings** → **Pages** 이동
- 상단에 다음 메시지가 보이면 성공:
  ```
  ✅ Your site is published at https://USERNAME.github.io/webcam-filter-wasm/
  ```

### 2️⃣ 웹사이트 접속 테스트

1. 배포된 URL 접속
2. 웹캠 권한 허용
3. 필터 버튼 클릭 테스트
4. 브라우저 콘솔 확인 (F12)
   - 에러 메시지가 없어야 합니다.
   - WASM 파일 로드 성공 메시지 확인

### 3️⃣ 다양한 브라우저 테스트

- ✅ Chrome (권장)
- ✅ Firefox
- ✅ Safari
- ✅ Edge

---

## 문제 해결

### ❌ 404 에러: 페이지를 찾을 수 없음

**원인**: GitHub Pages 설정이 제대로 안 됨

**해결 방법**:
1. Settings → Pages로 이동
2. Source가 `gh-pages` 브랜치로 설정되었는지 확인
3. 1~2분 대기 후 다시 접속

### ❌ WASM 파일 로드 실패

**증상**: 콘솔에 "Failed to load WASM" 에러

**원인**: `.nojekyll` 파일 누락

**해결 방법**:
```bash
# gh-pages 브랜치로 전환
git checkout gh-pages

# .nojekyll 파일 생성
touch .nojekyll

# 커밋 및 푸시
git add .nojekyll
git commit -m "Add .nojekyll file"
git push origin gh-pages

# main 브랜치로 복귀
git checkout main
```

### ❌ 웹캠이 작동하지 않음

**원인**: HTTPS가 아닌 HTTP로 접속

**해결 방법**:
- GitHub Pages는 자동으로 HTTPS를 제공합니다.
- 주소창에 `https://`로 시작하는지 확인
- 자동으로 리다이렉트되지 않으면 강제로 `https://` 입력

### ❌ 경로 오류: 파일을 찾을 수 없음

**증상**: CSS, JS 파일이 404 에러

**원인**: 절대 경로 문제

**해결 방법**:
`index.html`에서 경로 확인:

```html
<!-- ❌ 잘못된 경로 (절대 경로) -->
<link rel="stylesheet" href="/styles.css">

<!-- ✅ 올바른 경로 (상대 경로) -->
<link rel="stylesheet" href="./styles.css">
```

### ❌ GitHub Actions 빌드 실패

**증상**: Actions 탭에서 빌드 실패

**해결 방법**:
1. Actions 탭에서 실패한 워크플로우 클릭
2. 에러 메시지 확인
3. 주로 발생하는 문제:
   - Emscripten 설치 실패 → 워크플로우 파일 확인
   - 빌드 스크립트 권한 없음 → `chmod +x build.sh` 추가
   - emsdk 서브모듈 없음 → `submodules: true` 추가

---

## 배포 후 업데이트 방법

### 수동 배포 사용 시

```bash
# 1. 코드 수정 및 테스트
# ...

# 2. 빌드
./build.sh

# 3. main 브랜치 커밋
git add .
git commit -m "Update filters"
git push origin main

# 4. gh-pages 브랜치로 전환
git checkout gh-pages

# 5. 빌드 파일 복사
cp -r build/* .

# 6. 커밋 및 푸시
git add .
git commit -m "Update deployment"
git push origin gh-pages

# 7. main 브랜치로 복귀
git checkout main
```

### GitHub Actions 사용 시

```bash
# 1. 코드 수정 및 테스트
# ...

# 2. 빌드 (로컬 테스트용)
./build.sh

# 3. 커밋 및 푸시 (자동 배포됨!)
git add .
git commit -m "Update filters"
git push origin main

# 끝! GitHub Actions가 자동으로 빌드 및 배포
```

### Git Subtree 사용 시

```bash
# 1. 코드 수정 및 빌드
./build.sh

# 2. 빌드 결과 커밋
git add build/
git commit -m "Update build"

# 3. Subtree로 배포
git subtree push --prefix build origin gh-pages
```

---

## 🎯 추천 워크플로우

### 초보자에게 추천
**방법 1 (수동 배포)** → 간단하고 직관적, 실수 적음

### 중급자에게 추천
**방법 2 (GitHub Actions)** → 자동화로 편리함, 프로덕션 레벨

### 고급자에게 추천
**방법 3 (Git Subtree)** → 빠르고 효율적, Git 이해도 필요

---

## 📚 추가 자료

### 공식 문서
- [GitHub Pages 공식 가이드](https://docs.github.com/en/pages)
- [GitHub Actions 문서](https://docs.github.com/en/actions)
- [Emscripten 문서](https://emscripten.org/docs/)

### 유용한 링크
- [GitHub Pages 커스텀 도메인 설정](https://docs.github.com/en/pages/configuring-a-custom-domain-for-your-github-pages-site)
- [HTTPS 강제 설정](https://docs.github.com/en/pages/getting-started-with-github-pages/securing-your-github-pages-site-with-https)
- [WebAssembly MIME 타입 설정](https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types)

---

## 🎉 배포 완료!

축하합니다! 이제 여러분의 WebCam Filter WASM 프로젝트가 전 세계에 공개되었습니다!

**배포 URL 공유하기**:
```
https://USERNAME.github.io/webcam-filter-wasm/
```

**다음 단계**:
- ✨ 새로운 필터 추가해보기
- 🎨 UI 디자인 개선하기
- 📱 모바일 최적화하기
- 🚀 성능 벤치마크 측정하기

**문제가 있나요?**
- [GitHub Issues](../../issues)에 질문을 올려주세요!
- [GitHub Discussions](../../discussions)에서 토론해보세요!

---

**Happy Coding & Happy Deploying! 🚀**