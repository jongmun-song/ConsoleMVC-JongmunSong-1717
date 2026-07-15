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

    private:
        int m_sampleId;
        std::string m_name;
        double m_averageProductionTimePerUnit;
        double m_yieldRatio;
        ConsoleMVC::Model::QuantityGuard<int> m_stock;
    };
}
