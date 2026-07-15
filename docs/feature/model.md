# Model 설계 문서

## 1. 역할과 책임

Model은 애플리케이션의 상태(데이터)를 표현하고 보관하는 계층이다. `../CLAUDE.md`에 명시된 원칙에 따라 Model은:

- 콘솔 입출력을 직접 수행하지 않는다 (View의 책임).
- 사용자 입력을 직접 파싱하지 않는다 (Controller의 책임).
- S-Semi 도메인 타입(Sample, Order 등)을 포함하지 않는다 — 도메인 모델은 `SampleOrderSystem`에서 이 계층을 상속/구현하여 정의한다.

즉 이 문서가 정의하는 것은 **"모든 도메인 Model이 지켜야 할 공통 계약(contract)"**이며, 구체적인 필드나 상태 값이 아니다.

## 2. 참고: 소비 프로젝트가 다룰 데이터 형태 (요구사항 문서 근거)

`../ref/requirements.pdf`의 기능 명세를 보면, `SampleOrderSystem`이 이 Model 계층 위에 구현할 실제 데이터는 대략 다음과 같은 특징을 가진다 (구현 대상이 아니라, Model 계약이 지원해야 할 "형태"의 근거로만 참고):

- **엔티티형 데이터**: 고유 ID를 가지는 레코드 집합 (예: 시료 ID, 주문번호)
- **속성 집합**: 이름/속성으로 검색 가능한 필드들 (예: 시료명, 평균 생산시간, 수율)
- **상태를 가지는 레코드**: 유한한 상태 값 집합 사이를 전이하는 데이터 (예: RESERVED → CONFIRMED/PRODUCING/REJECTED, PRODUCING → CONFIRMED, CONFIRMED → RELEASE)
- **수량/재고형 데이터**: 증감이 가능한 수치형 필드 (예: 재고 수량)
- **대기열형 데이터**: FIFO로 처리되는 목록 (예: 생산 큐)

이 문서는 이러한 형태를 **일반화**하여, 아래와 같은 공통 Model 구성요소로 정의한다.

## 3. 공통 Model 구성요소

### 3.1 `IEntity` (또는 엔티티 베이스)
- 고유 식별자(ID)를 갖는 모든 도메인 객체의 최소 계약
- 요구사항: ID는 생성 시 부여되며 불변(immutable)이어야 한다.

### 3.2 `IModel<T>` / 리포지터리형 컬렉션 계약
- 엔티티 컬렉션에 대한 CRUD 접근을 추상화한 인터페이스
- 최소 연산: 추가(Add), 전체 조회(GetAll), ID 조회(GetById), 조건 검색(Find/Search), 수정(Update), 삭제(Remove — 필요 시)
- 검색(Find/Search)은 임의의 조건(predicate)을 받을 수 있도록 일반화하여, "이름으로 검색" 같은 구체 요구사항을 도메인 계층에서 조합할 수 있게 한다.
- 이 계층은 저장 방식(메모리, 파일, DB)에 대해 알지 못한다. 영속화는 `DataPersistence` 저장소의 책임이며, `IModel<T>`는 필요 시 영속성 어댑터를 주입받을 수 있는 구조(의존성 주입 지점)만 남겨둔다.

### 3.3 상태 보유 모델(Stateful Model) 확장 포인트
- 유한한 상태 집합 사이를 전이하는 도메인을 지원하기 위해, 상태 필드 + 상태 전이 검증 훅을 얹을 수 있는 확장 포인트를 제공한다.
- 이 계층 자체는 구체적인 상태 값(RESERVED 등)을 정의하지 않으며, "현재 상태", "허용된 다음 상태 집합을 검증하는 방법"만 계약으로 정의한다.

### 3.4 수량형 필드 헬퍼
- 증감 연산 시 음수로 내려가지 않도록 하는 등, 재고류 데이터에서 반복되는 불변식(invariant)을 검증하는 공통 유틸리티(예: `TryDecrease(amount)`가 실패를 반환하는 방식)를 제공한다. 실패 처리 정책(예외 vs 반환값)은 `controller.md`의 예외 처리 규칙과 일관되게 선택한다.

### 3.5 큐/대기열 모델
- FIFO 처리가 필요한 목록(예: 생산 큐)을 위한 일반화된 큐 래퍼를 제공한다. 표준 라이브러리 컨테이너(`std::queue`/`std::deque`)를 감싸 "다음 처리 대상 조회", "처리 완료 후 제거" 같은 도메인 친화적 접근자를 노출한다.

## 4. Model이 하지 않는 것 (Non-goals)

이 절은 **코어 계층**(`Model/` 디렉터리)을 기준으로 한다.

- 콘솔 출력 포맷팅 (View 책임)
- 사용자 입력 검증/파싱 (Controller 책임, 단 Model은 유효하지 않은 상태 전이 자체는 거부할 수 있다)
- 파일/DB I/O 구현 (`DataPersistence` 책임 — 이 저장소는 인터페이스만 노출)
- S-Semi 도메인 타입(Sample, Order 등) 정의 — 단, `Example/Model/`은 이 Non-goals의 예외적 검증(verification) 공간이다. §6 참고.

## 5. 디렉터리/네임스페이스 제안

```
Model/
  IEntity.h
  IModel.h          // 리포지터리형 컬렉션 계약
  StatefulModel.h    // 상태 전이 확장 포인트
  QuantityGuard.h    // 수량 불변식 헬퍼
  FifoQueue.h        // FIFO 대기열 래퍼
```

실제 도메인 엔티티(Sample, Order 등)는 이 디렉터리에 존재하지 않으며, `SampleOrderSystem`이 위 인터페이스를 구현하여 자신의 `Model/` 디렉터리에 정의한다.

## 6. PoC 검증용 예시 도메인 모델 (`Example/Model/`)

`../CLAUDE.md`의 "PoC 검증용 예시 도메인" 절에서 설명한 대로, 이 저장소는 코어 계약이 실제 도메인 형태를 지원하는지 검증하기 위해 `Example/Model/`에 `../ref/requirements.pdf` Chapter 2 기반의 예시 도메인(시료/주문/생산)을 둔다. 이 예시는 코어(§3)의 구성요소를 상속/조합하여 구현하며, 코어 자체는 수정하지 않는다.

아래 필드/형태는 참고용 예시이며, PDF 스스로도 "표기할 정보 수준은 자율적으로 결정"이라고 명시하고 있으므로 고정 스키마를 강제하는 것으로 읽지 않는다.

### 6.1 시료(Sample) — `IEntity` + `InMemoryModel<Sample>` + `QuantityGuard`
- 참고 필드: ID, 이름(시료명), 평균생산시간(분당 생산 개수 등 산정 근거), 수율(정상 생산 시료 / 총 생산 시료, 예: 0.9), 현재 재고 수량
- ID는 `IEntity`를 통해 부여, 컬렉션 보관은 코어의 `InMemoryModel<T>`를 그대로 사용(영속성 어댑터는 no-op/메모리 전용으로 주입)
- 재고 수량의 증감은 `QuantityGuard`로 음수 방지 등 불변식을 검증

### 6.2 주문(Order) — `IEntity` + `StatefulModel<OrderState>`
- 참고 필드: 시료 ID, 고객명, 주문 수량, 주문 상태
- 참고 상태 전이: `RESERVED` → `CONFIRMED`/`PRODUCING`/`REJECTED`, `PRODUCING` → `CONFIRMED`, `CONFIRMED` → `RELEASE` (예시 도메인에서 정의하는 구체 열거형이며, 코어의 `StatefulModel<TState>`는 이 열거형을 알지 못한다). 거절(`REJECTED`)은 접수된(`RESERVED`) 주문에 대해서만 수행하는 액션이며, `PRODUCING`/`CONFIRMED` 이후에는 거절 전이가 없다. `RELEASE`/`REJECTED`는 종단 상태다.
- 상태 전이 검증 훅에 위 전이 규칙을 주입하여 잘못된 전이를 거부하는 동작을 보여준다

### 6.3 생산(Production/ProductionQueueEntry) — `FifoQueue<T>` + `StatefulModel`
- 참고 필드: 순서(생산 큐 내 FIFO 순서), 주문번호, 시료, 주문량, 부족분, 실 생산량(`ceil(부족분 / 수율)`), 현재 상태(`PRODUCING` → `CONFIRMED` 전환 등)
- PDF는 "총 생산시간 = 평균생산시간 × 실생산량"이라는 계산만 명시하며 "예상 완료 시간" 같은 정확한 필드명을 강제하지 않는다 — 예시 도메인은 이 계산을 활용해 자유롭게 파생 필드를 정의할 수 있다
- 큐 자체는 코어의 `FifoQueue<T>`로 표현하고, 각 엔트리의 상태 전이는 `StatefulModel`로 표현한다

### 6.4 검증 방법
- 각 Phase(1/2/3)는 자신이 만든 범용 계약 위에 위 예시 도메인의 해당 부분을 얹어 실제 동작(엔티티 CRUD, 상태 전이 성공/실패, 큐 FIFO 순서 등)을 gtest로 확인한다. 자세한 작업 항목은 `docs/design/phase1.md` 이하 참고.
