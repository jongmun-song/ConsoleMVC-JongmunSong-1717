#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/feature/model.md section 6.1).
//
// Sample is a concrete IEntity<int> demonstrating that the core Model
// contract can express requirements.pdf Chapter 2's sample data: an
// identity, a small set of descriptive attributes, and a stock quantity
// guarded against going negative via the core QuantityGuard<int>.
//
// This type is a verification example, not a reusable dependency -
// SampleOrderSystem is free to define its own Sample type instead of
// reusing this one (see ../../CLAUDE.md "의존 강제 없음").
//
// Reserved-quantity tracking (docs/feature/model.md section 6.4, rule 4):
// approving an order does not immediately decrement stock. Instead, the
// portion of stock already promised to some order is tracked separately in
// m_reserved (another core QuantityGuard<int>, reused for the same
// never-goes-negative invariant as m_stock). "Available quantity" - the
// amount that can still be newly promised to an order - is always
// GetStockQuantity() - GetReservedQuantity(); actual stock is only ever
// decremented once a reservation is fulfilled (e.g. at shipment), via
// TryFulfillReservation.

#include "../../Model/IEntity.h"
#include "../../Model/QuantityGuard.h"

#include <string>
#include <utility>

namespace ConsoleMVC::Example::Model
{
    class Sample : public ConsoleMVC::Model::IEntity<int>
    {
    public:
        Sample(
            int sampleId,
            std::string name,
            double averageProductionTimePerUnit,
            double yieldRatio,
            int initialStockQuantity = 0)
            : m_sampleId(sampleId)
            , m_name(std::move(name))
            , m_averageProductionTimePerUnit(averageProductionTimePerUnit)
            , m_yieldRatio(yieldRatio)
            , m_stock(initialStockQuantity)
        {
        }

        int GetId() const override
        {
            return m_sampleId;
        }

        const std::string& GetName() const
        {
            return m_name;
        }

        // Average time (in whatever unit the caller chooses, e.g. minutes)
        // to produce a single unit of this sample.
        double GetAverageProductionTimePerUnit() const
        {
            return m_averageProductionTimePerUnit;
        }

        // Fraction of produced units that pass as good output, in [0.0, 1.0].
        double GetYieldRatio() const
        {
            return m_yieldRatio;
        }

        int GetStockQuantity() const
        {
            return m_stock.GetQuantity();
        }

        // Adds newly produced/received units to stock. Returns false (no
        // state change) if amount is negative.
        bool TryIncreaseStock(int amount)
        {
            return m_stock.TryIncrease(amount);
        }

        // Reserves/ships units out of stock. Returns false (no state
        // change) if amount is negative or exceeds current stock.
        bool TryDecreaseStock(int amount)
        {
            return m_stock.TryDecrease(amount);
        }

        // Amount of stock already promised to some order but not yet
        // shipped.
        int GetReservedQuantity() const
        {
            return m_reserved.GetQuantity();
        }

        // Stock that can still be newly promised to an order: actual stock
        // minus what is already reserved.
        int GetAvailableQuantity() const
        {
            return GetStockQuantity() - GetReservedQuantity();
        }

        // Promises `amount` units of currently available stock to an order,
        // without touching actual stock. Returns false (no state change) if
        // amount is negative or exceeds GetAvailableQuantity() - a caller
        // can never reserve more than is actually available right now (see
        // docs/feature/model.md section 6.4, rule 5 - no reserving against
        // stock that only exists in a future projection).
        bool TryReserve(int amount)
        {
            if (amount < 0 || amount > GetAvailableQuantity())
            {
                return false;
            }
            return m_reserved.TryIncrease(amount);
        }

        // Cancels a previously made reservation without shipping it (e.g. a
        // rolled-back approval), returning that stock to the available
        // pool. Returns false (no state change) if amount is negative or
        // exceeds the currently reserved quantity.
        bool TryReleaseReservation(int amount)
        {
            return m_reserved.TryDecrease(amount);
        }

        // Fulfils `amount` units of an existing reservation: decreases both
        // the reserved quantity and actual stock together (used when a
        // reserved order finally ships - see Example/Model/OrderApprovalService.h
        // Release()). Returns false (no state change) if amount is negative,
        // exceeds the reserved quantity, or (defensively) exceeds actual
        // stock.
        bool TryFulfillReservation(int amount)
        {
            if (amount < 0 || amount > GetReservedQuantity() || amount > GetStockQuantity())
            {
                return false;
            }

            if (!m_reserved.TryDecrease(amount))
            {
                return false;
            }

            if (!m_stock.TryDecrease(amount))
            {
                // Roll back the reservation decrease so a failed fulfilment
                // leaves state entirely unchanged.
                m_reserved.TryIncrease(amount);
                return false;
            }

            return true;
        }

    private:
        int m_sampleId;
        std::string m_name;
        double m_averageProductionTimePerUnit;
        double m_yieldRatio;
        ConsoleMVC::Model::QuantityGuard<int> m_stock;
        ConsoleMVC::Model::QuantityGuard<int> m_reserved;
    };
}
