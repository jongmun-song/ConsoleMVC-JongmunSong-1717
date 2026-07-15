#pragma once

// docs/feature/model.md section 3 (extensibility hooks) / docs/PLAN.md
// section 3 - Observer path.
//
// IModelObserver<T> lets any component (currently: nothing, in the future:
// a monitoring view or a separate DataMonitor process reading the same
// in-process Model) subscribe to entity lifecycle events without the Model
// needing to know who is listening.
//
// Each hook has a default no-op body so observers only override the events
// they actually care about (e.g. a monitor interested only in additions
// does not have to implement OnUpdated/OnRemoved).

namespace ConsoleMVC::Model
{
    template <typename T>
    class IModelObserver
    {
    public:
        virtual ~IModelObserver() = default;

        // Called after a new entity has been added to the Model.
        virtual void OnAdded(const T& entity) { (void)entity; }

        // Called after an existing entity has been replaced/updated,
        // including state transitions performed via StatefulModel.
        virtual void OnUpdated(const T& entity) { (void)entity; }

        // Called after an entity has been removed from the Model. The
        // entity value reflects its last known state before removal.
        virtual void OnRemoved(const T& entity) { (void)entity; }
    };
}
