#pragma once

// docs/feature/model.md section 3.2 / docs/PLAN.md section 3 - persistence
// adapter injection point.
//
// IModel<T> implementations must not know whether entities live only in
// memory, in a file, or in a database - that decision belongs to whichever
// adapter is injected here. This repository (ConsoleMVC) ships only the
// no-op NullPersistenceAdapter; a real file/DB-backed adapter is expected
// to live in the DataPersistence sibling repository and be injected at
// construction time (see InMemoryModel.h).

#include <vector>

namespace ConsoleMVC::Model
{
    template <typename T>
    class IPersistenceAdapter
    {
    public:
        virtual ~IPersistenceAdapter() = default;

        // Restores whatever entities were previously saved. Returns an
        // empty collection if there is nothing to restore.
        virtual std::vector<T> Load() const = 0;

        // Persists the given snapshot of entities, replacing whatever was
        // previously saved.
        virtual void Save(const std::vector<T>& entities) = 0;
    };

    // Memory-only stand-in: never restores anything and discards every
    // save request. Used as the default adapter so IModel<T> implementations
    // work without persistence until a real adapter is provided.
    template <typename T>
    class NullPersistenceAdapter : public IPersistenceAdapter<T>
    {
    public:
        std::vector<T> Load() const override { return {}; }
        void Save(const std::vector<T>& /*entities*/) override {}
    };
}
