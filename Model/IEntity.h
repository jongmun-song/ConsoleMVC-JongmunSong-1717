#pragma once

// docs/feature/model.md section 3.1 - IEntity
//
// Minimal contract shared by every domain object that this MVC skeleton can
// store in an IModel<T>: it has a unique identifier that is assigned once
// (typically at construction time) and never changes afterwards. Concrete
// domain types (defined by consumers such as SampleOrderSystem) implement
// this interface; this repository does not know what an ID "means" beyond
// being a stable, comparable value.
//
// The ID type is a template parameter (defaults to int) so that consumers
// can use whatever identifier shape fits their domain (numeric sample IDs,
// string order numbers, etc.) without this skeleton hardcoding either.

namespace ConsoleMVC::Model
{
    template <typename TId = int>
    class IEntity
    {
    public:
        using IdType = TId;

        virtual ~IEntity() = default;

        // Returns the entity's unique, immutable identifier. Implementations
        // must guarantee that the returned value never changes over the
        // lifetime of the entity - callers (e.g. IModel<T>) rely on GetId()
        // as a stable key.
        virtual TId GetId() const = 0;
    };
}
