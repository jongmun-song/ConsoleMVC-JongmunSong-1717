# PRD — ConsoleMVC

## 1. 배경

`SSemiProductManager` 과제는 "반도체 시료 생산주문관리 시스템"(`SampleOrderSystem`)을 구현하기 전, 4개의 PoC(`ConsoleMVC`, `DataPersistence`, `DataMonitor`, `DummyDataGenerator`)로 핵심 기능을 검증하도록 요구한다(`../ref/requirements.pdf` Chapter 3, [미션1]).

`ConsoleMVC`는 그중 **"MVC 스켈레톤 코드"** PoC로, "Model / Controller / View 패키지 구조와 역할 분리 완성"을 목표로 한다. 자세한 저장소 역할 구분과 재사용 제약은 `../CLAUDE.md`에 정의되어 있으며, 본 문서는 그 내용을 바탕으로 프로젝트 전체 요구사항을 정리한다.

## 2. 목적

콘솔 기반 애플리케이션을 위한 **재사용 가능한 MVC 골격 모듈**을 제공한다. 이 모듈은 특정 비즈니스 도메인(반도체 시료, 주문 등)에 종속되지 않으며, `SampleOrderSystem`을 포함한 어떤 콘솔 프로젝트에서도 헤더/소스를 그대로 include하여 사용할 수 있어야 한다.

## 3. 범위

### 포함 (In Scope)
- Model / View / Controller 각 계층의 책임 분리 및 기본 클래스(또는 인터페이스) 설계
- 여러 단계로 중첩 가능한 콘솔 메뉴 내비게이션 구조 (메인 메뉴 → 서브메뉴 → 뒤로가기/종료)
- 목록 조회, 페이지네이션, 입력값 확인(Y/N), 상태 배지 등 반복적으로 쓰이는 콘솔 UI 패턴의 재사용 가능한 표현 방식
- 잘못된 입력, 취소, 종료 등 컨트롤러 레벨의 공통 예외/흐름 처리 규칙

### 제외 (Out of Scope)
- 시료(Sample), 주문(Order), 생산 라인(ProductionLine) 등 S-Semi 도메인 모델과 그 상태 전이(RESERVED/CONFIRMED/PRODUCING/RELEASE/REJECTED)
- 데이터 영속성 구현(`DataPersistence` 저장소 담당)
- 실시간 데이터 모니터링 도구(`DataMonitor` 저장소 담당)
- 더미 데이터 생성(`DummyDataGenerator` 저장소 담당)

도메인 예시(시료 관리, 주문 승인/거절, 모니터링, 출고 처리, 생산 라인 등)는 `SampleOrderSystem`이 이 모듈 위에 실제로 구현할 화면들이며, 본 저장소에서는 "이런 형태의 화면/흐름을 일반화된 방식으로 지원할 수 있는가"를 검증하기 위한 참고 사례로만 사용한다.

## 4. 대상 소비자(Consumers)

| 저장소 | 이 모듈 사용 방식 |
|---|---|
| `SampleOrderSystem` | Model/View/Controller 베이스를 상속/확장하여 실제 메뉴(시료 관리, 주문, 모니터링, 출고 처리, 생산 라인)를 구현 |
| `DataMonitor` | 필요 시 동일한 콘솔 UI 패턴(목록/페이지네이션)을 재사용 가능 |
| `DummyDataGenerator` | 콘솔 기반 도구 UI가 필요하다면 View 계층 재사용 가능 |

## 5. 기술 스택

- 언어/표준: C++20 (`stdcpp20`)
- 빌드: Visual Studio (`ConsoleMVC.slnx`, `ConsoleMVC.vcxproj`), MSBuild, Win32 `Application` (콘솔 서브시스템), x86/x64, Debug/Release
- 외부 의존성 없음 (표준 라이브러리만 사용, 모듈 이식성을 위해)

## 6. 핵심 설계 원칙

1. **도메인 무관성** — 도메인 타입/상태를 하드코딩하지 않는다.
2. **이식성** — 절대 경로, 프로젝트 전용 전역 상태를 두지 않는다. 다른 프로젝트가 소스/헤더를 그대로 include해서 빌드할 수 있어야 한다.
3. **역할 분리** — Model은 상태만, View는 출력만, Controller는 입력 처리와 흐름 제어만 담당한다(자세한 내용은 `docs/feature/model.md`, `docs/feature/view.md`, `docs/feature/controller.md` 참고).
4. **확장성** — 새로운 메뉴/화면을 추가할 때 기존 코드 수정 없이 Controller/메뉴 트리에 항목을 추가하는 방식으로 확장 가능해야 한다(개방-폐쇄 원칙).

## 7. 산출물

- Model 계층 설계 문서: `docs/feature/model.md`
- View 계층 설계 문서: `docs/feature/view.md`
- Controller 계층 설계 문서: `docs/feature/controller.md`
- MVC 스켈레톤 소스 코드 (Model/View/Controller 디렉터리 구조 및 예제 와이어링)

## 8. 완료 기준 (Definition of Done)

- Model/View/Controller가 명확히 분리된 디렉터리/네임스페이스 구조로 존재
- 최소 2단계 이상 중첩된 메뉴 내비게이션이 동작하는 예제 포함
- 잘못된 입력에 대한 재입력 유도, 뒤로가기, 종료 처리가 일관된 규칙으로 구현됨
- 다른 프로젝트(`SampleOrderSystem`)가 도메인 모델만 추가하면 즉시 사용할 수 있는 수준의 일반화
