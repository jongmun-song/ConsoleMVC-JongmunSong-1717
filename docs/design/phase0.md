# Phase 0 — 프로젝트 골격 준비

## 목표

Model/View/Controller 코드를 작성하기 전에, 디렉터리 구조와 빌드 시스템 등록 방식, 네이밍 컨벤션을 확정한다. 이 Phase는 기능 코드를 포함하지 않으며, 이후 모든 Phase가 따를 "틀"을 만드는 단계다.

## 근거 문서

- `../feature/model.md`, `../feature/view.md`, `../feature/controller.md`의 "디렉터리 제안" 절
- `../../CLAUDE.md` — 이식성 제약(다른 프로젝트가 헤더/소스를 그대로 include 가능해야 함)

## 작업 항목

1. **디렉터리 구조 생성**
   ```
   Model/
   View/
   Controller/
   ```
   (각 디렉터리에는 Phase 1~3에서 실제 헤더/소스가 채워진다. Phase 0에서는 빈 디렉터리 또는 최소한의 네임스페이스 선언 헤더만 존재해도 된다.)
2. **네임스페이스 컨벤션 확정**: 예) `ConsoleMVC::Model`, `ConsoleMVC::View`, `ConsoleMVC::Controller`. 최종 확정된 네임스페이스는 이후 모든 Phase 문서/코드에서 동일하게 사용한다.
3. **빌드 등록 방식 확인**: `ConsoleMVC.vcxproj` / `ConsoleMVC.vcxproj.filters`에 새 디렉터리의 파일을 추가하는 절차를 한 번 시험 삼아 확인한다(예: 빈 헤더 1개를 추가해보고 MSBuild 빌드가 되는지 확인).
4. **gtest 연동 확인**: `packages.config`에 등록된 gtest(1.7.0)가 실제로 링크/실행 가능한 상태인지 최소한의 더미 테스트(`TEST(Sanity, AlwaysTrue)`)로 확인한다. 이후 Phase의 테스터 작업이 여기서 막히지 않도록 하기 위함이다.

## 완료 기준

- `Model/`, `View/`, `Controller/` 디렉터리가 저장소에 존재하고 빌드 설정에 등록되어 있다.
- 네임스페이스 컨벤션이 문서(본 파일 또는 별도 커밋 메시지)에 기록되어 있다.
- 더미 테스트 1개가 gtest로 빌드/실행되어 통과한다.

## 리뷰/테스트 포인트

- **코드리뷰어**: 이 단계는 실질적 로직이 없으므로 디렉터리/네임스페이스 명명이 `docs/feature/*.md`와 일치하는지만 확인한다.
- **테스터**: gtest 실행 파이프라인 자체가 동작하는지만 확인한다(더미 테스트 통과 여부).

## Out of scope

- Model/View/Controller의 실제 인터페이스 구현 (Phase 1~3)
- 도메인 로직, 예제 데이터
