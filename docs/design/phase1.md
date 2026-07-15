# Phase 1 — Model 계층 구현

## 목표

`docs/feature/model.md`에 정의된 공통 Model 계약을 실제로 구현한다. 이 Phase의 산출물은 도메인 타입(Sample, Order 등)을 포함하지 않는 **범용 계약과 참조 구현**이며, 데이터는 메모리 상에만 존재해도 무방하다(파일 저장은 선택 사항).

이 Phase에서 가장 중요한 것은 **나중에 `DataMonitor`가 붙을 수 있는 확장 지점**을 함께 설계하는 것이다 (`docs/PLAN.md` 3절 참고).

## 근거 문서

- `../feature/model.md` 전체

## 작업 항목

1. **`IEntity`**: 고유 ID를 갖는 엔티티의 최소 계약. ID는 생성 시 부여되고 이후 불변.
2. **`IModel<T>`**: 리포지터리형 컬렉션 계약.
   - 연산: `Add`, `GetAll`, `GetById`, `Find(predicate)`, `Update`, (필요 시)`Remove`
   - 참조 구현은 메모리 컨테이너(`std::vector`/`std::unordered_map`) 기반으로 작성한다. 파일 저장이 필요하다고 판단되면 별도 어댑터로 분리하되, 지금 파일 저장을 강제하지는 않는다.
3. **`StatefulModel` 확장 포인트**: 상태 필드 + "허용된 다음 상태인지 검증"하는 훅. 구체 상태 값(열거형 등)은 이 계층에서 정의하지 않고, 검증 함수를 주입받는 형태(예: `std::function<bool(State,State)>` 또는 순수 가상 함수)로 일반화한다.
4. **`QuantityGuard`**: 수량 증감 시 음수로 내려가지 않도록 검증하는 헬퍼. 실패 시 예외를 던질지 `bool`/`std::optional`을 반환할지는 `docs/feature/controller.md`의 예외 처리 정책(정상적으로 실패할 수 있는 결과로 취급)과 일관되게 **반환값 기반**으로 설계할 것을 권장한다.
5. **`FifoQueue`**: FIFO 대기열 래퍼. `Enqueue`, `PeekNext`, `DequeueCompleted` 등 도메인 친화적 접근자를 제공하되 내부는 표준 컨테이너를 감싼다.
6. **확장성 훅 (모니터링 대비)**
   - **`IModelObserver`**: 엔티티 추가/수정/상태 전이 시 알림을 받을 수 있는 관찰자 인터페이스. `IModel<T>`가 `Subscribe(IModelObserver*)` / `Unsubscribe(...)`를 제공한다.
   - **읽기 전용 조회 인터페이스**: `IModel<T>`의 `GetAll`/`GetById`/`Find`는 Controller뿐 아니라 향후 별도 컴포넌트(예: 같은 프로세스 내 모니터링 뷰, 또는 나중에 `DataMonitor`가 참고할 스냅샷 함수)에서도 독립적으로 호출 가능해야 한다 — 즉 Controller를 거치지 않고도 Model 상태를 읽을 수 있는 공개 API여야 한다.
   - **영속성 어댑터 주입 지점**: `IModel<T>`가 저장 방식(메모리 vs 파일)에 대해 알지 못하도록, 실제 저장/복원 로직은 별도 인터페이스(`IPersistenceAdapter<T>`, 선택적으로 주입)로 분리해 둔다. 지금은 no-op(메모리 전용) 구현체 하나만 있어도 충분하다 — 핵심은 나중에 파일/DB 기반 어댑터로 교체하거나, `DataMonitor`가 같은 어댑터를 통해 데이터를 읽어갈 수 있는 자리를 만들어 두는 것이다.

7. **PoC 예시 도메인 검증** (다음 위임에서 진행 — 아직 코드 작성 전)
   - 위 1~6번 항목(`IEntity`/`IModel`/`StatefulModel`/`QuantityGuard`/`FifoQueue`/확장성 훅)은 이미 구현/리뷰/테스트가 완료되었으므로 재작업하지 않는다.
   - 이번 항목은 그 위에 `../ref/requirements.pdf` 기반 예시 도메인(시료/주문/생산)을 `Example/Model/`에 구현하여, 코어 계약이 실제 도메인 형태를 표현 가능함을 검증하는 후속 작업이다(`../feature/model.md` §6 참고).
   - 시료는 `IEntity` + `InMemoryModel<Sample>` + `QuantityGuard`, 주문은 `IEntity` + `StatefulModel<OrderState>`, 생산은 `FifoQueue<T>` + `StatefulModel`로 표현한다.
   - gtest로 각 예시 도메인 타입의 CRUD/상태 전이/큐 동작을 검증한다.
   - 코어(`Model/`)는 수정하지 않는다.

## 완료 기준

- 위 6개 구성요소가 `Model/` 디렉터리에 구현되어 있고 컴파일된다. (완료)
- Observer 등록/알림이 최소 한 개의 예시(더미 관찰자)로 동작을 확인할 수 있다. (완료)
- 도메인 타입 하드코딩이 없다(제네릭/템플릿 또는 순수 가상 인터페이스로 작성). (완료, 코어 기준)
- (다음 위임) 7번 항목: `Example/Model/`의 예시 도메인이 코어 계약으로 표현되고 gtest로 검증된다.

## 리뷰 포인트 (code-reviewer)

- Model이 콘솔 출력이나 입력 파싱을 하지 않는지 (SRP, 계층 책임 분리)
- 확장 지점(Observer, 영속성 어댑터)이 실제로 "교체 가능"한 형태인지, 아니면 이름만 있고 하드와이어드되어 있지 않은지
- 이름/역할이 명확한지 (예: `IModel<T>`의 각 메서드가 단일 책임을 갖는지)

## 테스트 포인트 (tester)

- `IModel<T>`의 CRUD 동작 (추가 후 조회, ID로 조회, predicate 검색)
- `StatefulModel`의 유효/무효 상태 전이 케이스
- `QuantityGuard`의 경계값(0, 음수 시도) 케이스
- `FifoQueue`의 순서 보장(FIFO) 케이스
- Observer 등록 후 변경 시 알림이 오는지, 해제 후에는 알림이 오지 않는지
- (다음 위임, 7번 항목) `Example/Model/`의 시료/주문/생산 예시가 각각 `QuantityGuard`/`StatefulModel`/`FifoQueue`를 통해 기대한 동작을 보이는지

## Out of scope

- 코어 계층(`Model/`)에 도메인 엔티티(Sample, Order)를 정의하는 것 — 단, `Example/Model/`의 PoC 검증용 예시 구현(7번 항목)은 범위에 포함된다
- 실제 파일/DB 영속성 구현 (어댑터 인터페이스 자리만 마련, `Example/`도 메모리 전용)
