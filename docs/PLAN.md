# PLAN — ConsoleMVC 구현 계획

## 1. 구현 수준 정의

이번 구현에서 목표로 하는 완성도는 다음과 같다 (`docs/PRD.md`의 완료 기준을 구체화):

- **파일로 구성된 Model / Controller / View 패키지 구조와 역할 분리를 완성**한다. 즉, 디렉터리/네임스페이스가 계층별로 분리되어 있고, 각 계층이 자신의 책임(`docs/feature/model.md`, `docs/feature/view.md`, `docs/feature/controller.md`)만 수행하는 실행 가능한 코드가 있어야 한다.
- **데이터 저장 방식은 자유**다 — 파일로 저장해도 되고, 프로그램 메모리에만 존재해도 된다. `DataPersistence` 저장소 수준의 영속성(재시작 후 복구 등)을 이 저장소에서 완성할 필요는 없다.
- 단, **나중에 별도의 데이터 모니터링 모듈(`DataMonitor`)을 붙일 수 있도록 확장성을 열어둔다.** 이는 지금 모니터링 기능을 구현하라는 뜻이 아니라, Model 계층이 외부 관찰자에게 상태를 노출할 수 있는 지점(변경 알림, 읽기 전용 조회 인터페이스, 영속성 어댑터 주입 지점)을 설계 단계에서 마련해 두라는 뜻이다.
- **PoC 컨셉 강화**: 코어 계층(`Model/`, `View/`, `Controller/`)은 계속 도메인 무관하게 유지하되, 각 Phase에서 그 계층이 만든 범용 계약이 실제로 동작하는지 눈으로 확인할 수 있도록 `Example/`(`ConsoleMVC::Example` 네임스페이스)에 `../ref/requirements.pdf` 기반의 예시 도메인(시료/주문/생산)을 함께 쌓는다. `Example/`의 데이터는 메모리에만 존재하며 영속성을 갖지 않는다(`../CLAUDE.md`의 "PoC 검증용 예시 도메인" 절 참고). 단, 이미 구현/리뷰/테스트가 끝난 Phase 0/Phase 1 코어 산출물은 재작업하지 않고, 그 위에 `Example/` 계층을 추가로 쌓아 나간다.

## 2. 참고 문서

- `../CLAUDE.md` — 저장소 목적 및 도메인 무관성 제약
- `docs/PRD.md` — 전체 목표, 범위, 완료 기준
- `docs/feature/model.md` / `docs/feature/view.md` / `docs/feature/controller.md` — 계층별 세부 설계
- `docs/CODE_CONVENTION.md` — 클린 코드 기준
- `.claude/agents/*.md` — manager/developer/code-reviewer/tester 역할 정의. 아래 각 Phase는 이 에이전트 팀의 작업 단위(위임 → 리뷰 → 테스트)와 1:1로 대응하도록 설계했다.

## 3. 확장성 설계 원칙 (모니터링 대비)

향후 `DataMonitor`가 이 모듈에 붙을 수 있는 방식은 크게 두 가지이며, 이번 구현은 두 경로 모두를 막지 않도록 설계한다.

1. **관찰자(Observer) 경로** — 같은 프로세스 내에서든, 향후 다른 계층에서든 Model의 변경 이벤트(추가/수정/상태 전이)를 구독할 수 있는 인터페이스(`IModelObserver` 등)를 Model 계층에 마련한다. 지금 당장 실제 구독자는 없어도 되지만, 등록/해제 지점 자체는 존재해야 한다.
2. **조회 전용 스냅샷 경로** — Model이 현재 상태를 읽기 전용으로 조회할 수 있는 인터페이스(`GetAll`, `GetById` 등, `docs/feature/model.md`의 `IModel<T>` 계약)를 Controller/View와 무관하게 독립적으로 호출할 수 있어야 한다. 향후 별도 프로세스(모니터링 도구)가 붙을 경우를 대비해, 이 조회 경로가 특정 영속성 구현(메모리 vs 파일)에 종속되지 않도록 `IPersistenceAdapter` 같은 주입 지점을 남겨둔다.

두 경로 모두 지금 이 저장소에서 "완전히 구현"하는 것이 아니라, **나중에 갈아끼우거나 확장할 수 있는 인터페이스 자리**를 비워두는 것이 이번 단계의 목표다.

## 4. 개발 단계 (Phase) 개요

| Phase | 문서 | 목표 |
|---|---|---|
| Phase 0 | `docs/design/phase0.md` | 프로젝트 골격 준비 — 디렉터리 구조, 빌드 등록, 네임스페이스/네이밍 컨벤션 확정 |
| Phase 1 | `docs/design/phase1.md` | Model 계층 구현 — 엔티티/컬렉션 계약 + 확장성 훅(Observer, 스냅샷, 영속성 어댑터 주입 지점). 해당 계층의 예시 도메인(시료/주문/생산 Model, `Example/Model/`) 검증 포함 |
| Phase 2 | `docs/design/phase2.md` | View 계층 구현 — 콘솔 렌더링 컴포넌트 (입출력 미결합). 해당 계층의 예시 도메인 화면(`Example/View/`) 검증 포함 |
| Phase 3 | `docs/design/phase3.md` | Controller 계층 구현 — 메뉴 트리, 내비게이션 루프, 입력/예외 처리 규칙. 해당 계층의 예시 메뉴 트리(`Example/Controller/`) 검증 포함 |
| Phase 4 | `docs/design/phase4.md` | 통합 데모 및 확장성 검증 — Model/View/Controller 와이어링, 모니터링 확장 지점 시연, 예시 도메인 전체 통합 데모, 최종 리뷰/테스트 |

각 Phase는 아래 순서로 진행한다 (에이전트 팀 워크플로):

1. `manager`가 해당 Phase 문서를 기준으로 `developer`에게 구현을 위임한다.
2. `developer`가 구현을 완료하면 `code-reviewer`가 `docs/CODE_CONVENTION.md` 기준으로 리뷰한다.
3. `tester`가 해당 Phase의 산출물에 대해 gtest 기반 테스트를 작성/실행한다.
4. 위 세 단계가 통과하면 다음 Phase로 진행한다. Phase는 순차 의존성이 있으므로(Model → View → Controller → 통합) 이전 Phase의 산출물이 불안정한 상태로 다음 Phase를 시작하지 않는다.

## 5. Phase 간 의존 관계

```
Phase 0 (골격)
   └─▶ Phase 1 (Model)
          └─▶ Phase 2 (View)   ※ Model에 독립적이지만 Phase 1의 디렉터리/네이밍 컨벤션을 따름
                 └─▶ Phase 3 (Controller)  ※ Model + View를 모두 사용
                        └─▶ Phase 4 (통합/확장성 검증)
```

View(Phase 2)는 Model 상태를 직접 참조하지 않으므로 이론적으로 Phase 1과 병렬 진행이 가능하지만, 동일한 네이밍/디렉터리 컨벤션을 Phase 0~1에서 확정한 뒤 시작하는 것을 권장한다.
