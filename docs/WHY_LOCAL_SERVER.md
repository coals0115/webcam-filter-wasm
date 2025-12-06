# 왜 로컬 서버가 필요한가?

> `file://`로 HTML을 열면 안 되고, `http://localhost`로 열어야 하는 이유

---

## 핵심 개념: "출처(Origin)"

브라우저는 모든 웹페이지에 **"출처(Origin)"**를 부여한다.

```
출처 = 프로토콜 + 도메인 + 포트
```

### 출처 예시

| URL | 출처 |
|-----|------|
| `http://localhost:8080/index.html` | `http://localhost:8080` |
| `http://localhost:3000/index.html` | `http://localhost:3000` |
| `https://google.com/page.html` | `https://google.com` |
| `file:///Users/.../index.html` | **"opaque origin" (불투명 출처)** |

---

## 문제: `file://`은 출처가 없다

MDN 웹 문서에 이렇게 나와 있다:

> **"Modern browsers usually treat the origin of files loaded using the `file:///` scheme as opaque origins."**
>
> (현대 브라우저는 `file:///`로 로드된 파일의 출처를 "불투명 출처"로 취급한다)

### "불투명 출처(Opaque Origin)"란?

- 브라우저가 **"넌 누군지 모르겠어"**라고 하는 것
- 출처가 `null`로 설정됨
- **같은 폴더에 있는 파일도 "다른 출처"로 취급됨**

```
file:///Users/musinsa/build/index.html   → 출처: null (불투명)
file:///Users/musinsa/build/filters.wasm → 출처: null (불투명)

브라우저: "둘 다 null인데... null ≠ null이야. 차단!"
```

### 실제 에러 메시지

```
Access to fetch at 'file:///...filters.wasm' from origin 'null'
has been blocked by CORS policy
```

---

## 왜 이런 정책을 만들었나?

**보안 때문이다.**

만약 `file://`에서 자유롭게 파일을 불러올 수 있으면:

```
1. 해커가 악성 HTML 파일을 만듦
2. 너한테 이메일로 보냄: "이거_열어봐.html"
3. 너가 더블클릭해서 열면 file:// 프로토콜로 열림
4. 그 HTML 안의 JavaScript가 file:///Users/너/Documents/비밀번호.txt 를 읽음
5. 읽은 내용을 해커 서버로 전송
```

이런 공격을 막으려고 **"file://은 다른 파일을 불러올 수 없다"**는 정책을 만든 것이다.

---

## 서버를 띄우면 왜 되나?

```bash
python3 -m http.server 8080
```

이 명령어를 실행하면 `build/` 폴더의 파일들이 HTTP로 서빙된다:

```
build/index.html    → http://localhost:8080/index.html
build/filters.wasm  → http://localhost:8080/filters.wasm
build/filters.js    → http://localhost:8080/filters.js
```

이제 모든 파일이 **같은 출처**를 갖게 된다:

```
http://localhost:8080/index.html   → 출처: http://localhost:8080
http://localhost:8080/filters.wasm → 출처: http://localhost:8080

브라우저: "둘 다 http://localhost:8080이네. 같은 출처니까 OK!"
```

---

## 동일 출처 정책 (Same-Origin Policy)

브라우저의 기본 보안 정책이다.

> **"같은 출처의 리소스만 자유롭게 접근할 수 있다"**

### 같은 출처 조건 (3가지 모두 일치해야 함)

1. **프로토콜** (http, https, file 등)
2. **도메인** (localhost, google.com 등)
3. **포트** (8080, 3000 등)

### 예시

| 기준 | 비교 대상 | 같은 출처? |
|------|----------|-----------|
| `http://localhost:8080` | `http://localhost:8080/other.js` | O (같음) |
| `http://localhost:8080` | `http://localhost:3000/other.js` | X (포트 다름) |
| `http://localhost:8080` | `https://localhost:8080/other.js` | X (프로토콜 다름) |
| `http://localhost:8080` | `http://google.com:8080/other.js` | X (도메인 다름) |

---

## 정리

| 방식 | 출처 | 결과 |
|------|------|------|
| `file://`로 열기 | `null` (불투명) | 같은 폴더 파일도 차단됨 |
| `http://localhost`로 열기 | `http://localhost:8080` | 같은 서버 파일은 허용됨 |

**Python 서버는 "출처를 만들어주는 역할"을 한다.**

- `file://`은 출처가 없어서(`null`) 뭘 해도 차단당함
- `http://`는 출처가 있어서 같은 출처끼리는 자유롭게 통신 가능

---

## 참고: 다른 로컬 서버 옵션

Python 외에도 아무 HTTP 서버나 사용 가능하다:

```bash
# Python
python3 -m http.server 8080

# Node.js (npx)
npx serve

# Node.js (http-server)
npx http-server

# PHP
php -S localhost:8080
```

핵심은 **HTTP 프로토콜로 서빙하는 것**이다.

---

## 참고 자료

- [MDN: Same-origin policy](https://developer.mozilla.org/en-US/docs/Web/Security/Same-origin_policy)
- [MDN: CORS](https://developer.mozilla.org/en-US/docs/Web/HTTP/CORS)
- [MDN: File origins](https://developer.mozilla.org/en-US/docs/Web/Security/Same-origin_policy#file_origins)
