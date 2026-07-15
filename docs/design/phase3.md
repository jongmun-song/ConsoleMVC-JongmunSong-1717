# Phase 3 — Controller 계층 구현

## 목표

`docs/feature/controller.md`에 정의된 메뉴 내비게이션 구조와 예외 처리 규칙을 구현한다. Controller는 Phase 1(Model)과 Phase 2(View)를 사용하는 첫 번째 계층이다.

## 근거 문서

- `../feature/controller.md` 전체

## 작업 항목

1. **`IController` / `MenuNode`**: 표시 라벨, 실행 핸들러, 부모 메뉴 참조를 갖는 컴포지트 메뉴 트리. `ReadOnlyMenuNode`(조회 전용)와 `ActionMenuNode`(상태 변경 액션)를 구분할 수 있는 타입을 제공한다.
2. **`NavigationLoop`**: 렌더링 → 입력 대기 → 유효성 검사 → 실행 → 전이(유지/하위 진입/상위 복귀/종료) 루프.
3. **`InputReader`**: 표준입력 한 줄 읽기 + 트림 유틸리티.
4. **예외/오류 처리 규칙 구현** (`docs/feature/controller.md` 4절의 표를 그대로 구현):
   - 존재하지 않는 메뉴 번호 → 오류 메시지 + 재입력
   - 숫자가 아닌 입력 → 재입력
   - 빈 입력 → 재입력
   - Y/N 외 확인 응답 → 재요청
   - Model이 거부한 상태 전이 → 실패 메시지 + 메뉴 복귀 (크래시 금지)
   - 예기치 않은 런타임 예외 → 최상위에서 포착, 메인 메뉴로 복귀
5. **최소 데모 메뉴 트리**: 도메인 데이터 없이(또는 더미 Entity 1~2개 정도로) 2단계 이상 중첩된 메뉴(예: 메인 메뉴 → 서브메뉴 → 목록 조회/뒤로가기)가 실제로 동작하는 것을 보여주는 예제. 이 예제는 어디까지나 Controller 구조를 검증하기 위한 것이며, Phase 4에서 Model/View와 함께 더 완성된 형태로 통합된다.
6. **PoC 예시 메뉴 트리 검증**: `Example/Controller/`에 예시 메뉴 트리(메인 메뉴 → 시료 관리/시료 주문/주문 승인·거절/모니터링/생산라인/출고 처리)를 `IController`/`MenuNode`/`NavigationLoop`로 구현하여, `Example/Model/`(Phase 1)과 `Example/View/`(Phase 2)를 실제로 연결한다(`../feature/controller.md` §7 참고). 코어(`Controller/`)는 수정하지 않는다.

## 완료 기준

- `Controller/` 디렉터리에 위 구성요소가 구현되어 있고 컴파일된다.
- 2단계 이상 중첩 메뉴가 실제로 콘솔에서 동작한다(수동 실행 또는 테스트로 확인).
- `docs/feature/controller.md` 4절의 6가지 예외 상황이 모두 재현 가능하고, 프로그램이 비정상 종료되지 않는다.
- (6번 항목) `Example/Controller/`의 예시 메뉴 트리가 `Example/Model/`, `Example/View/`를 연결하여 실제로 동작한다.

## 리뷰 포인트 (code-reviewer)

- 메뉴 트리 확장 시 기존 `NavigationLoop` 코드를 수정할 필요가 없는 구조인지(개방-폐쇄 원칙)
- 입력 파싱/검증 로직이 여러 곳에 중복되어 있지 않은지
- 확인 취소(N) 시 Model에 어떤 변경도 가해지지 않는지

## 테스트 포인트 (tester)

- `docs/feature/controller.md` 4절의 예외 케이스 표를 그대로 gtest 케이스로 변환하여 커버
- 메뉴 전이(하위 진입/상위 복귀/종료) 상태 기계가 올바르게 동작하는지
- 확인 취소 시 부작용 없음(Model 상태 불변) 검증
- (6번 항목) `Example/Controller/`의 예시 메뉴 트리에서 주문 승인/거절 등 상태 변경 액션 취소 시 `Example/Model/` 상태가 불변인지

## Out of scope

- `SampleOrderSystem`이 실제로 소비할 완성된 도메인 메뉴 애플리케이션 — `Example/Controller/`의 예시 메뉴 트리(6번 항목)는 검증용 데모일 뿐, `SampleOrderSystem`이 그대로 재사용해야 하는 것은 아니다
- 모니터링 확장 지점 자체 구현(이미 Phase 1에서 완료) — 이 Phase에서는 Controller가 그 지점을 침범하지 않는지만 확인
