#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/model.md section 6.2).
//
// Order is a concrete IEntity<int> that demonstrates the core
// StatefulModel<TState> extension point with a real (but example-only)
// state machine drawn from requirements.pdf Chapter 2:
//
//   RESERVED  -> CONFIRMED | PRODUCING | REJECTED
//   PRODUCING -> CONFIRMED
//   CONFIRMED -> RELEASE
//   RELEASE, REJECTED are terminal - no further transitions are allowed.
//
// Rejection (-> REJECTED) is only ever performed from RESERVED, matching
// requirements.pdf Chapter 2 ("주문 거절: 접수된 특정 주문에 대해 거절").
//
// OrderState is defined here, in the example domain, precisely because the
// core (Model/StatefulModel.h) must stay agnostic of what "RESERVED" means.
//
// This type is a verification example, not a reusable dependency -
// SampleOrderSystem is free to define its own Order type instead of
// reusing this one (see ../../CLAUDE.md "의존 강제 없음").

#include "../../Model/IEntity.h"
#include "../../Model/StatefulModel.h"

#include <string>
#include <utility>

namespace ConsoleMVC::Example::Model
{
    enum class OrderState
    {
        RESERVED,
        CONFIRMED,
        PRODUCING,
        RELEASE,
        REJECTED
    };

    class Order : public ConsoleMVC::Model::IEntity<int>
    {
    public:
        Order(int orderId, int sampleId, std::string customerName, int orderedQuantity)
            : m_orderId(orderId)
            , m_sampleId(sampleId)
            , m_customerName(std::move(customerName))
            , m_orderedQuantity(orderedQuantity)
            , m_state(OrderState::RESERVED, &Order::IsTransitionAllowed)
        {
        }

        int GetId() const override
        {
            return m_orderId;
        }

        int GetSampleId() const
        {
            return m_sampleId;
        }

        const std::string& GetCustomerName() const
        {
            return m_customerName;
        }

        int GetOrderedQuantity() const
        {
            return m_orderedQuantity;
        }

        OrderState GetState() const
        {
            return m_state.GetState();
        }

        // Attempts to move this order to `nextState`. Returns false (state
        // unchanged) if the transition is not allowed - see the transition
        // table documented above IsTransitionAllowed.
        bool TryTransitionTo(OrderState nextState)
        {
            return m_state.TryTransition(nextState);
        }

    private:
        static bool IsTransitionAllowed(const OrderState& current, const OrderState& next)
        {
            switch (current)
            {
            case OrderState::RESERVED:
                return next == OrderState::CONFIRMED
                    || next == OrderState::PRODUCING
                    || next == OrderState::REJECTED;
            case OrderState::PRODUCING:
                return next == OrderState::CONFIRMED;
            case OrderState::CONFIRMED:
                return next == OrderState::RELEASE;
            case OrderState::RELEASE:
            case OrderState::REJECTED:
            default:
                return false;
            }
        }

        int m_orderId;
        int m_sampleId;
        std::string m_customerName;
        int m_orderedQuantity;
        ConsoleMVC::Model::StatefulModel<OrderState> m_state;
    };
}
