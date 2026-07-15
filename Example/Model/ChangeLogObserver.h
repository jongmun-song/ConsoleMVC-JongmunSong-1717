#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/PLAN.md section 3 - Observer extension point / docs/design/phase4.md
// item 2 "모니터링 확장 지점 시연").
//
// ChangeLogObserver<T> is a spike, not a real monitor: it exists only to
// prove that Model/IModelObserver.h's Subscribe/Unsubscribe hook (already
// shipped by Phase 1's InMemoryModel<T>) is enough for a component outside
// Controller/View to learn about entity lifecycle events without either of
// those layers being modified. A future DataMonitor process would plug in
// the same way this class does.
//
// It stays domain-agnostic on purpose (it is templated on T and takes a
// `Describe` callback rather than hard-coding Sample/Order field names) even
// though it lives under Example/ - the "entity label" and "how to render one
// entity as text" are supplied by whoever constructs it (see
// ExampleAppContext.h), so the same observer type could log any IEntity<T>
// collection.

#include "../../Model/IModelObserver.h"

#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::Example::Model
{
    template <typename T>
    class ChangeLogObserver : public ConsoleMVC::Model::IModelObserver<T>
    {
    public:
        using DescribeFn = std::function<std::string(const T&)>;

        // `entityLabel` names the kind of entity being watched (e.g.
        // "Sample") purely for readability of the log lines; `describe`
        // renders one entity as a short human-readable string.
        ChangeLogObserver(std::string entityLabel, DescribeFn describe)
            : m_entityLabel(std::move(entityLabel))
            , m_describe(std::move(describe))
        {
        }

        void OnAdded(const T& entity) override
        {
            Record("added", entity);
        }

        void OnUpdated(const T& entity) override
        {
            Record("updated", entity);
        }

        void OnRemoved(const T& entity) override
        {
            Record("removed", entity);
        }

        // Every log line recorded so far, in order. Exposed so a caller
        // (or a future test) can verify notifications were received without
        // scraping stdout.
        const std::vector<std::string>& GetEntries() const
        {
            return m_entries;
        }

    private:
        void Record(const char* verb, const T& entity)
        {
            std::string line = "[ChangeLog] " + m_entityLabel + " " + verb + ": " + m_describe(entity);
            m_entries.push_back(line);
            std::cout << line << std::endl;
        }

        std::string m_entityLabel;
        DescribeFn m_describe;
        std::vector<std::string> m_entries;
    };
}
