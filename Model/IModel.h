#pragma once

// docs/feature/model.md section 3.2 - IModel<T>
//
// Repository-style CRUD contract for a collection of entities. This
// interface does not know how entities are stored (memory, file, DB); it
// only defines the operations that any Model consumer (Controller today,
// a future DataMonitor tomorrow) can rely on:
//
//   Add / GetAll / GetById / Find(predicate) / Update / Remove
//
// GetAll/GetById/Find are read-only and intentionally independent of
// Controller - see docs/PLAN.md section 3 ("read-only snapshot path"): any
// component holding an IModel<T>* can query current state without going
// through a Controller.
//
// T must satisfy IEntity<TId> (i.e. expose a nested `IdType` and a
// `GetId() const` accessor), which is exactly what IEntity<TId> provides.

#include "IModelObserver.h"

#include <functional>
#include <optional>
#include <vector>

namespace ConsoleMVC::Model
{
    template <typename T>
    class IModel
    {
    public:
        using IdType = typename T::IdType;
        using Predicate = std::function<bool(const T&)>;

        virtual ~IModel() = default;

        // Adds a new entity to the collection. Behavior when an entity with
        // the same ID already exists is left to the implementation.
        virtual void Add(const T& entity) = 0;

        // Returns a snapshot of every entity currently in the collection.
        virtual std::vector<T> GetAll() const = 0;

        // Returns the entity with the given ID, or std::nullopt if none
        // exists.
        virtual std::optional<T> GetById(const IdType& id) const = 0;

        // Returns every entity for which predicate(entity) is true.
        virtual std::vector<T> Find(const Predicate& predicate) const = 0;

        // Replaces the stored entity that shares entity.GetId() with the
        // given value. Returns false if no such entity exists.
        virtual bool Update(const T& entity) = 0;

        // Removes the entity with the given ID. Returns false if no such
        // entity exists.
        virtual bool Remove(const IdType& id) = 0;

        // Registers an observer to receive OnAdded/OnUpdated/OnRemoved
        // notifications. The caller retains ownership of the observer and
        // must Unsubscribe before destroying it. Any IModel<T> consumer -
        // Controller, DataMonitor, etc. - can subscribe through this
        // interface without knowing the concrete storage implementation.
        virtual void Subscribe(IModelObserver<T>* observer) = 0;

        // Stops notifying the given observer. No-op if it was never
        // subscribed.
        virtual void Unsubscribe(IModelObserver<T>* observer) = 0;
    };
}
