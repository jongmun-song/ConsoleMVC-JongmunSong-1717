#pragma once

// docs/feature/model.md section 3.5 - queue/waitlist model
//
// FifoQueue<T> wraps std::deque<T> to expose domain-friendly accessors for
// first-in-first-out processing (e.g. a production queue), without this
// skeleton knowing anything about what T represents.
//
// PeekNext returns std::optional<T> instead of throwing/asserting on an
// empty queue, consistent with the "expected failure is a normal result"
// policy shared with StatefulModel and QuantityGuard.

#include <deque>
#include <optional>

namespace ConsoleMVC::Model
{
    template <typename T>
    class FifoQueue
    {
    public:
        // Adds an item to the back of the queue.
        void Enqueue(const T& item)
        {
            m_items.push_back(item);
        }

        // Returns the next item to be processed (the front of the queue)
        // without removing it, or std::nullopt if the queue is empty.
        std::optional<T> PeekNext() const
        {
            if (IsEmpty())
            {
                return std::nullopt;
            }

            return m_items.front();
        }

        // Removes the front item once its processing has completed.
        // Returns false (no-op) if the queue was already empty.
        bool DequeueCompleted()
        {
            if (IsEmpty())
            {
                return false;
            }

            m_items.pop_front();
            return true;
        }

        bool IsEmpty() const
        {
            return m_items.empty();
        }

        std::size_t Size() const
        {
            return m_items.size();
        }

    private:
        std::deque<T> m_items;
    };
}
