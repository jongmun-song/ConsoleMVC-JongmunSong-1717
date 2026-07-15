# CLAUDE.md

이 파일은 이 저장소에서 작업할 때 Claude Code(claude.ai/code)에게 제공되는 가이드입니다.

## 저장소 목적

이 저장소는 `../ref/requirements.pdf`에 기술된 "반도체 시료 생산주문관리 시스템" 과제의 **"MVC 스켈레톤 코드" PoC**입니다.

`SSemiProductManager/` 아래의 5개 형제 PoC 저장소 중 하나입니다.
- `ConsoleMVC` (본 저장소) — Model/Controller/View 구조 및 역할 분리
- `DataPersistence` — 저장소/CRUD 처리 (파일, JSON, 또는 DB)
- `DataMonitor` — 저장된 데이터를 실시간으로 조회하는 콘솔 도구
- `DummyDataGenerator` — 영속성 계층에 더미 데이터를 채워 넣는 도구
- `SampleOrderSystem` — 실제 비즈니스 애플리케이션(주문/생산/재고 흐름)으로, 본 저장소의 MVC 골격을 모듈로 가져다 사용할 것으로 예상됨

**핵심 설계 제약: 이 저장소의 재사용 코어(`Model/`, `View/`, `Controller/`)는 도메인에 종속되지 않아야 합니다.** 이 코어는 범용 콘솔 MVC 배관(메뉴/화면 내비게이션, 입력 처리, Model/View/Controller 기본 타입 및 연결 구조)을 제공하는 것이지, S-Semi의 비즈니스 로직(Sample, Order, ProductionLine 등)을 제공하는 것이 아닙니다. `SampleOrderSystem`이 바로 그 도메인 로직을 이 스켈레톤 위에 구축하는 곳입니다. 구체적으로:
- S-Semi 전용 타입(Sample, Order, RESERVED/CONFIRMED 등의 상태)을 코어(`Model/`, `View/`, `Controller/` 루트 디렉터리)에 하드코딩하지 마세요 — 이는 이를 사용하는 프로젝트에 속합니다. 단, 아래 "PoC 검증용 예시 도메인" 절에서 설명하는 `Example/` 아래에는 검증 목적의 참고 구현을 둘 수 있습니다.
- Model/View/Controller 코어는 하나로 고정된 메뉴 트리가 아니라, 어떤 콘솔 앱이든 상속·확장할 수 있는 재사용 가능한 기본 클래스/인터페이스(예: 템플릿 또는 가상 기본 클래스)로 유지하세요.
- 하드코딩된 절대 경로, 프로젝트 전용 전역 변수, 이 저장소 안에서만 의미가 있는 요소는 피하세요 — 다른 프로젝트의 빌드가 이 모듈의 헤더/소스를 직접 include할 수 있어야 합니다.

## PoC 검증용 예시 도메인 (`Example/`)

이 저장소는 "MVC 스켈레톤 코드" PoC이므로, 코어 계층이 만든 범용 계약이 실제 도메인 형태를 제대로 지원하는지 각 Phase마다 눈으로 확인할 필요가 있다. 이를 위해 코어와는 명확히 분리된 별도 디렉터리 `Example/`(`Example/Model/`, `Example/View/`, `Example/Controller/`, 네임스페이스 `ConsoleMVC::Example`)에 `../ref/requirements.pdf` Chapter 2 기반의 구체 도메인(시료/주문/생산) 예시 구현을 둔다.

- **목적**: "각 Phase가 만든 범용 계약이 실제 도메인 형태를 제대로 지원하는지 검증하는 참고/데모 구현"이며, 완성된 비즈니스 애플리케이션이 아니다.
- **분리 원칙**: `Example/`의 코드는 코어(`Model/`, `View/`, `Controller/`)를 상속/조합해서 만들 뿐, 코어를 수정하지 않는다. 코어와 `Example/`이 서로 섞이지 않아야 하며, 코어 헤더는 `Example/`의 존재를 알지 못한다.
- **의존 강제 없음**: `SampleOrderSystem`이 이 `Example/` 코드를 반드시 재사용해야 하는 것은 아니다. 패턴 참고는 가능하나 의존을 강제하지 않으며, `SampleOrderSystem`은 자신만의 도메인 타입을 별도로 정의할 수 있다.
- **영속성 없음**: `Example/`의 데이터는 메모리에만 존재한다(예: 코어의 `InMemoryModel<T>` 그대로 사용). 파일 저장/영속성은 구현하지 않는다 — 이는 `DataPersistence` 저장소의 몫이다.

## 현재 상태

프로젝트는 현재 소스 파일이 없는 빈 Visual Studio C++ 콘솔 앱 스캐폴드입니다 — `ConsoleMVC.vcxproj` 끝의 비어 있는 `<ItemGroup>`과 `.vcxproj.filters`의 비어 있는 필터 그룹에서 확인할 수 있습니다. 첫 소스 파일을 추가할 때는 `ConsoleMVC.vcxproj`(`<ClCompile>`/`<ClInclude>`가 포함된 `<ItemGroup>`)와 `ConsoleMVC.vcxproj.filters`(“소스 파일” / “헤더 파일”로 분류) 양쪽 모두에 등록하세요.

## 빌드

Visual Studio C++ 프로젝트(`ConsoleMVC.vcxproj` / `ConsoleMVC.slnx`)이며, C++20(`stdcpp20`)을 대상으로 `x86`/`x64`, Debug/Release 구성의 Win32 `Application`(콘솔 서브시스템)으로 빌드됩니다.

- `ConsoleMVC.slnx`를 Visual Studio(v145 툴셋)에서 열어 빌드/실행하거나,
- 다음과 같이 명령줄에서 MSBuild로 빌드할 수 있습니다:
  ```
  msbuild ConsoleMVC.vcxproj /p:Configuration=Debug /p:Platform=x64
  ```
- 빌드된 실행 파일은 `x64\Debug\ConsoleMVC.exe`에서 실행하세요(경로는 구성/플랫폼에 따라 다름).

아직 이 저장소에는 테스트 러너나 린트 설정이 없습니다. 첫 소스 파일을 추가할 때 테스트를 도입한다면 함께 구성하세요.

## 아키텍처 가이드 (requirements.pdf Chapter 3 — PoC 범위 기준)

MVC 스켈레톤은 `SampleOrderSystem`이 이 모듈을 수정하지 않고도 실제 메뉴 체계(시료 관리 / 주문 접수·승인·거절 / 모니터링 / 출고 처리 / 생산 라인)를 그 위에 쌓을 수 있도록, 역할이 명확히 분리되어 있음을 보여줘야 합니다:

- **Model** — 상태를 보유/표현하며, 콘솔 입출력을 하지 않습니다.
- **View** — 오직 콘솔 출력 렌더링만 담당합니다(요구사항 문서에서도 화면 레이아웃은 구현자가 자유롭게 결정하도록 반복해서 명시하고 있으며, PDF의 예시 화면은 참고용일 뿐 강제 사항이 아닙니다).
- **Controller** — 콘솔 입력을 읽고 Model 업데이트와 View 렌더링을 구동하며, 메뉴 내비게이션 루프(메인 메뉴 → 서브메뉴 → 뒤로가기)를 소유합니다.

최종적으로 이 모듈을 사용할 앱은 메인 메뉴 → 서브메뉴(예: 시료 관리 → 시료 등록/조회/검색)로 이어지는 다단계 메뉴 구조이므로, Controller/메뉴 추상화는 단일 계층의 flat switch가 아니라 임의 깊이의 중첩 메뉴와 번호 선택 입력 루프를 지원하도록 설계하세요.
