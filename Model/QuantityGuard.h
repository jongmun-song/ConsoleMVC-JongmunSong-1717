#pragma once

// docs/feature/model.md section 3.4 - quantity field helper
//
// QuantityGuard<TQuantity> wraps a single numeric field and enforces the
// invariant that it never drops below zero. It is intentionally generic
// (no "stock"/"inventory" naming) so that any quantity-like domain field
// can reuse it.
//
// Consistent with docs/feature/controller.md section 4 ("Model-level
// invalid operations are an ordinary failure, not an exception"),
// TryIncrease/TryDecrease report success via a bool return value instead of
// throwing. A failed call leaves the guarded quantity unchanged.

namespace ConsoleMVC::Model
{
    template <typename TQuantity = int>
    class QuantityGuard
    {
    public:
        explicit QuantityGuard(TQuantity initialQuantity = TQuantity{})
            : m_quantity(initialQuantity)
        {
        }

        TQuantity GetQuantity() const
        {
            return m_quantity;
        }

        // Increases the quantity by `amount`. Rejects negative amounts so
        // that "increase" and "decrease" stay unambiguous operations.
        // Returns false (no state change) if amount is negative.
        bool TryIncrease(TQuantity amount)
        {
            if (amount < TQuantity{})
            {
                return false;
            }

            m_quantity = m_quantity + amount;
            return true;
        }

        // Decreases the quantity by `amount`. Returns false (no state
        // change) if amount is negative or would take the quantity below
        // zero.
        bool TryDecrease(TQuantity amount)
        {
            if (amount < TQuantity{} || amount > m_quantity)
            {
                return false;
            }

            m_quantity = m_quantity - amount;
            return true;
        }

    private:
        TQuantity m_quantity;
    };
}
