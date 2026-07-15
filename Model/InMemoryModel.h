#pragma once

// docs/feature/model.md section 3.2 - reference implementation of IModel<T>.
//
// InMemoryModel<T> keeps entities in an unordered_map keyed by their ID and
// notifies subscribed IModelObserver<T> instances on every Add/Update/Remove.
// A persistence adapter can optionally be injected (see IPersistenceAdapter.h);
// by default it uses NullPersistenceAdapter<T>, so this class works purely
// in memory unless a real adapter is supplied by the caller.
//
// This class contains no console I/O and no domain-specific logic - it only
// knows about T through the IEntity<TId> contract (T::IdType, T::GetId()).

#include "IModel.h"
#include "IModelObserver.h"
#include "IPersistenceAdapter.h"

#include <algorithm>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ConsoleMVC::Model
{
    template <typename T>
    class InMemoryModel : public IModel<T>
    {
    public:
        using IdType = typename IModel<T>::IdType;
        using Predicate = typename IModel<T>::Predicate;

        explicit InMemoryModel(
            std::shared_ptr<IPersistenceAdapter<T>> persistenceAdapter =
                std::make_shared<NullPersistenceAdapter<T>>())
            : m_persistenceAdapter(std::move(persistenceAdapter))
        {
            for (const T& restoredEntity : m_persistenceAdapter->Load())
            {
                m_entitiesById.insert_or_assign(restoredEntity.GetId(), restoredEntity);
            }
        }

        void Add(const T& entity) override
        {
            // insert_or_assign (rather than operator[]) avoids requiring T
            // to be default-constructible - IEntity implementations are not
            // required to provide one.
            m_entitiesById.insert_or_assign(entity.GetId(), entity);
            PersistSnapshot();
            NotifyObservers(&IModelObserver<T>::OnAdded, entity);
        }

        std::vector<T> GetAll() const override
        {
            std::vector<T> allEntities;
            allEntities.reserve(m_entitiesById.size());
            for (const auto& [id, entity] : m_entitiesById)
            {
                allEntities.push_back(entity);
            }
            return allEntities;
        }

        std::optional<T> GetById(const IdType& id) const override
        {
            const auto found = m_entitiesById.find(id);
            if (found == m_entitiesById.end())
            {
                return std::nullopt;
            }
            return found->second;
        }

        std::vector<T> Find(const Predicate& predicate) const override
        {
            std::vector<T> matches;
            for (const auto& [id, entity] : m_entitiesById)
            {
                if (predicate(entity))
                {
                    matches.push_back(entity);
                }
            }
            return matches;
        }

        bool Update(const T& entity) override
        {
            const auto found = m_entitiesById.find(entity.GetId());
            if (found == m_entitiesById.end())
            {
                return false;
            }

            m_entitiesById.insert_or_assign(entity.GetId(), entity);
            PersistSnapshot();
            NotifyObservers(&IModelObserver<T>::OnUpdated, entity);
            return true;
        }

        bool Remove(const IdType& id) override
        {
            const auto found = m_entitiesById.find(id);
            if (found == m_entitiesById.end())
            {
                return false;
            }

            const T removedEntity = found->second;
            m_entitiesById.erase(found);
            PersistSnapshot();
            NotifyObservers(&IModelObserver<T>::OnRemoved, removedEntity);
            return true;
        }

        // Registers an observer to receive OnAdded/OnUpdated/OnRemoved
        // notifications. The caller retains ownership of the observer and
        // must Unsubscribe before destroying it.
        void Subscribe(IModelObserver<T>* observer) override
        {
            if (observer == nullptr)
            {
                return;
            }
            m_observers.push_back(observer);
        }

        // Stops notifying the given observer. No-op if it was never
        // subscribed.
        void Unsubscribe(IModelObserver<T>* observer) override
        {
            m_observers.erase(
                std::remove(m_observers.begin(), m_observers.end(), observer),
                m_observers.end());
        }

    private:
        void PersistSnapshot()
        {
            m_persistenceAdapter->Save(GetAll());
        }

        void NotifyObservers(
            void (IModelObserver<T>::* notification)(const T&), const T& entity)
        {
            for (IModelObserver<T>* observer : m_observers)
            {
                (observer->*notification)(entity);
            }
        }

        std::unordered_map<IdType, T> m_entitiesById;
        std::vector<IModelObserver<T>*> m_observers;
        std::shared_ptr<IPersistenceAdapter<T>> m_persistenceAdapter;
    };
}
