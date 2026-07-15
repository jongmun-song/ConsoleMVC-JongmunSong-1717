#pragma once

// PoC verification example (see ../../CLAUDE.md "PoC 검증용 예시 도메인 (Example/)"
// and docs/design/phase4.md item 3 - "영속성 어댑터 교체 가능성 확인").
//
// AppendOnlyFilePersistenceAdapter<T> is a second IPersistenceAdapter<T>
// implementation (the first being Model/IPersistenceAdapter.h's
// NullPersistenceAdapter<T>) that proves the adapter injection point
// InMemoryModel<T> already exposes (see Model/InMemoryModel.h's
// constructor) is real: swapping which adapter an InMemoryModel<T> is built
// with changes nothing about IModel<T>/Controller/View - only construction.
//
// It is intentionally minimal, matching docs/design/phase4.md's guidance
// that a full file-backed restore is out of scope for this repository (that
// belongs to the DataPersistence sibling repository):
//   - Save() appends one line per call (a timestamped snapshot marker plus
//     one serialized line per entity) to a text file, rather than
//     overwriting it - "append-only" journal, not a compacted snapshot.
//   - Load() always returns an empty collection; this adapter does not
//     parse its own file back into T. Demonstrating "an adapter can be
//     swapped in without touching IModel<T>/Controller/View" does not
//     require round-tripping data, only that Save()/Load() are called
//     through the same IPersistenceAdapter<T> contract.
//
// Like ChangeLogObserver.h, this stays domain-agnostic: it is templated on T
// and takes a `Serialize` callback rather than hard-coding any entity's
// fields, so the same adapter type works for Sample, Order, or any other
// IEntity<TId>.

#include "../../Model/IPersistenceAdapter.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace ConsoleMVC::Example::Model
{
    template <typename T>
    class AppendOnlyFilePersistenceAdapter : public ConsoleMVC::Model::IPersistenceAdapter<T>
    {
    public:
        using SerializeFn = std::function<std::string(const T&)>;

        AppendOnlyFilePersistenceAdapter(std::string filePath, SerializeFn serialize)
            : m_filePath(std::move(filePath))
            , m_serialize(std::move(serialize))
        {
        }

        // Never restores anything - see the class comment above for why a
        // full parse-back is out of this PoC's scope.
        std::vector<T> Load() const override
        {
            return {};
        }

        // Appends the current snapshot as a new journal block instead of
        // overwriting the file, so every past Save() call remains on disk.
        void Save(const std::vector<T>& entities) override
        {
            std::ofstream file(m_filePath, std::ios::app);
            if (!file.is_open())
            {
                return;
            }

            file << "-- snapshot at " << CurrentTimestamp() << " (" << entities.size() << " entities) --\n";
            for (const T& entity : entities)
            {
                file << m_serialize(entity) << '\n';
            }
        }

    private:
        static std::string CurrentTimestamp()
        {
            const std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm localTime{};
            localtime_s(&localTime, &now);

            char buffer[32];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
            return std::string(buffer);
        }

        std::string m_filePath;
        SerializeFn m_serialize;
    };
}
