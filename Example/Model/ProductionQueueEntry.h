#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/model.md section 6.3).
//
// ProductionQueueEntry demonstrates the core FifoQueue<T> and
// StatefulModel<TState> extension points for requirements.pdf Chapter 2's
// production step: an order's shortage (ordered quantity minus available
// stock) is turned into an actual production quantity via the sample's
// yield ratio, and the resulting total production time is derived from the
// sample's average production time per unit.
//
// FIFO order among queued entries is owned by the core FifoQueue<T> wrapper
// (see Model/FifoQueue.h) - this type does not track its own queue
// position.
//
// This type is a verification example, not a reusable dependency -
// SampleOrderSystem is free to define its own production entry type instead
// of reusing this one (see ../../CLAUDE.md "의존 강제 없음").

#include "../../Model/IEntity.h"
#include "../../Model/StatefulModel.h"

#include <cmath>

namespace ConsoleMVC::Example::Model
{
    // Number of units that must actually be produced to cover a shortage,
    // given the sample's yield ratio (fraction of good units per unit
    // produced). Returns 0 for a non-positive shortage or an invalid
    // (non-positive) yield ratio, since no production is meaningful there.
    inline int CalculateActualProductionQuantity(int shortageQuantity, double yieldRatio)
    {
        if (shortageQuantity <= 0 || yieldRatio <= 0.0)
        {
            return 0;
        }

        return static_cast<int>(std::ceil(shortageQuantity / yieldRatio));
    }

    // Total production time = average production time per unit * the
    // actual quantity that must be produced.
    inline double CalculateTotalProductionTime(
        double averageProductionTimePerUnit, int actualProductionQuantity)
    {
        return averageProductionTimePerUnit * static_cast<double>(actualProductionQuantity);
    }

    // Production progress for a single queued entry. PRODUCING is the entry
    // state while it waits/executes in the queue; CONFIRMED marks that
    // production has completed and the covering order can proceed (see
    // Order::TryTransitionTo(OrderState::CONFIRMED)).
    enum class ProductionState
    {
        PRODUCING,
        CONFIRMED
    };

    class ProductionQueueEntry : public ConsoleMVC::Model::IEntity<int>
    {
    public:
        // sampleYieldRatio/sampleAverageProductionTimePerUnit are read from
        // the covering Sample at the moment this entry is created; the
        // derived quantity/time fields are computed once here rather than
        // re-read from the Sample every time they are queried.
        ProductionQueueEntry(
            int orderId,
            int sampleId,
            int orderedQuantity,
            int shortageQuantity,
            double sampleYieldRatio,
            double sampleAverageProductionTimePerUnit)
            : m_orderId(orderId)
            , m_sampleId(sampleId)
            , m_orderedQuantity(orderedQuantity)
            , m_shortageQuantity(shortageQuantity)
            , m_actualProductionQuantity(
                  CalculateActualProductionQuantity(shortageQuantity, sampleYieldRatio))
            , m_totalProductionTime(
                  CalculateTotalProductionTime(
                      sampleAverageProductionTimePerUnit, m_actualProductionQuantity))
            , m_state(ProductionState::PRODUCING, &ProductionQueueEntry::IsTransitionAllowed)
        {
        }

        // Identifies this entry by the order it covers - an order has at
        // most one production queue entry outstanding at a time.
        int GetId() const override
        {
            return m_orderId;
        }

        int GetSampleId() const
        {
            return m_sampleId;
        }

        int GetOrderedQuantity() const
        {
            return m_orderedQuantity;
        }

        int GetShortageQuantity() const
        {
            return m_shortageQuantity;
        }

        int GetActualProductionQuantity() const
        {
            return m_actualProductionQuantity;
        }

        double GetTotalProductionTime() const
        {
            return m_totalProductionTime;
        }

        ProductionState GetState() const
        {
            return m_state.GetState();
        }

        // Marks production as complete. Returns false (state unchanged) if
        // called on an entry that is not currently PRODUCING.
        bool TryTransitionTo(ProductionState nextState)
        {
            return m_state.TryTransition(nextState);
        }

    private:
        static bool IsTransitionAllowed(const ProductionState& current, const ProductionState& next)
        {
            return current == ProductionState::PRODUCING && next == ProductionState::CONFIRMED;
        }

        int m_orderId;
        int m_sampleId;
        int m_orderedQuantity;
        int m_shortageQuantity;
        int m_actualProductionQuantity;
        double m_totalProductionTime;
        ConsoleMVC::Model::StatefulModel<ProductionState> m_state;
    };
}
