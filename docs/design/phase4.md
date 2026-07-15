# Phase 4 — 통합 데모 및 확장성 검증

## 목표

Phase 1~3에서 만든 Model/View/Controller를 하나의 최소 데모 애플리케이션(`main.cpp` 등)으로 와이어링하고, `docs/PLAN.md` 3절에서 설계한 모니터링 확장 지점이 실제로 "붙일 수 있는" 상태인지 검증한다. 이 Phase가 끝나면 저장소는 `docs/PRD.md`의 완료 기준을 모두 충족해야 한다.

## 근거 문서

- `docs/PRD.md` 8절 (완료 기준)
- `docs/PLAN.md` 3절 (확장성 설계 원칙)
- `docs/feature/model.md`, `view.md`, `controller.md`

## 작업 항목

1. **통합 와이어링**: Phase 1~3에서 각각 만든 `Example/Model/`, `Example/View/`, `Example/Controller/`의 예시 도메인(시료/주문/생산)을 하나의 콘솔 애플리케이션 진입점(`main.cpp` 등)으로 최종 연결한다. 이 데모는 도메인 검증용이며, `SampleOrderSystem`이 그대로 가져다 쓸 코드가 아니라 "코어 계약이 실제 도메인 구조에서도 동작함"을 보여주는 참조 예제임을 주석/문서에 명시한다. 데이터는 메모리에만 존재하며 영속성은 구현하지 않는다.
2. **모니터링 확장 지점 시연**: Phase 1에서 만든 `IModelObserver`를 구현하는 더미 관찰자(예: 변경 로그를 콘솔이나 별도 파일에 남기는 간단한 구현)를 하나 작성하여, Model에 등록했을 때 Controller/View 코드를 전혀 수정하지 않고도 변경 알림을 받을 수 있음을 보여준다. 이 더미 관찰자는 실제 `DataMonitor`가 아니라 "확장 지점이 유효하다"는 것을 증명하기 위한 스파이크 코드다.
3. **영속성 어댑터 교체 가능성 확인**: Phase 1의 `IPersistenceAdapter<T>` 자리에 메모리 전용 구현체가 아닌 다른 구현체(예: 단순 텍스트 파일에 append하는 구현)로 교체해도 `IModel<T>`/Controller/View 코드가 변경되지 않는지 확인한다(완전한 파일 영속성을 구현하라는 뜻은 아니며, "교체 가능함"을 보여줄 정도의 최소 구현이면 충분하다).
4. **최종 문서 정합성 확인**: `docs/PRD.md`의 완료 기준 5가지를 모두 재확인한다.
   - Model/View/Controller 디렉터리 분리 ✅ (Phase 0~3)
   - 2단계 이상 중첩 메뉴 동작 ✅ (Phase 3)
   - 예외 처리 규칙 일관성 ✅ (Phase 3)
   - 도메인 모델만 추가하면 즉시 사용 가능한 일반화 수준 ← 이 Phase에서 최종 검증
   - `Example/`의 예시 도메인(시료/주문/생산)이 각 계층 계약을 실제로 사용해 동작함 ← 이 Phase에서 통합 데모(1번 항목)로 최종 검증

## 완료 기준

- 통합 데모가 빌드/실행되며, 최소 2단계 메뉴 내비게이션과 하나 이상의 상태 변경 액션(확인 프롬프트 포함)을 눈으로 확인할 수 있다.
- 더미 Observer가 Model 변경 시 알림을 수신하는 것이 테스트 또는 실행 로그로 확인된다.
- 영속성 어댑터를 교체해도 Model/Controller/View의 공개 인터페이스가 변경되지 않는다.
- `docs/PRD.md`의 완료 기준(5가지, `Example/` 검증 기준 포함)이 모두 충족된다.
- `Example/Model/`, `Example/View/`, `Example/Controller/`가 하나의 진입점에서 통합되어 시료 등록 → 주문 접수 → 승인/거절 → 생산 → 출고 정도의 최소 흐름을 시연한다.

## 리뷰 포인트 (code-reviewer)

- 예시 도메인 코드가 `Example/`에 격리되어 있고, 코어(`Model/`, `View/`, `Controller/`)의 "도메인 무관성" 제약을 어기지 않는지(코어에 도메인 값이 스며들지 않았는지)
- 확장 지점(Observer, 영속성 어댑터)이 데모를 위해 특별히 우회되거나 하드코딩되지 않았는지

## 테스트 포인트 (tester)

- 통합 시나리오 테스트(엔티티 추가 → 상태 변경 → Observer 알림 수신까지 end-to-end)
- 영속성 어댑터를 교체한 상태에서도 기존 Model 테스트(Phase 1)가 그대로 통과하는지(회귀 확인)

## Out of scope

- `DataMonitor` 저장소 자체의 구현 (별도 저장소)
- `SampleOrderSystem`의 실제 도메인 화면 구현 (별도 저장소)
