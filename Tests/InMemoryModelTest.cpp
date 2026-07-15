// Phase 1-A - IModel<T> / InMemoryModel<T> contract tests.
//
// docs/design/phase1.md "테스트 포인트 (tester)" section for the 1-A scope:
//   - IModel<T> CRUD behavior (add-then-query, GetById, Find(predicate))
//   - Observer registration/notification and Unsubscribe stopping it
//
// StatefulModel / QuantityGuard / FifoQueue are covered separately in
// Tests/StatefulModelTest.cpp, Tests/QuantityGuardTest.cpp and
// Tests/FifoQueueTest.cpp (Phase 1-B scope).
//
// The dummy entity below is intentionally domain-agnostic (no Sample/Order
// naming) - it exists solely to exercise the generic IEntity<TId>/IModel<T>
// contract.

#include <gtest/gtest.h>

#include "../Model/IEntity.h"
#include "../Model/IModel.h"
#include "../Model/IModelObserver.h"
#include "../Model/InMemoryModel.h"

#include <string>

namespace
{
    using ConsoleMVC::Model::IEntity;
    using ConsoleMVC::Model::IModel;
    using ConsoleMVC::Model::IModelObserver;
    using ConsoleMVC::Model::InMemoryModel;

    // Minimal domain-agnostic entity used only to exercise IEntity<TId>.
    class TestEntity : public IEntity<int>
    {
    public:
        TestEntity(int id, std::string label)
            : m_id(id), m_label(std::move(label))
        {
        }

        int GetId() const override { return m_id; }
        const std::string& GetLabel() const { return m_label; }

    private:
        int m_id;
        std::string m_label;
    };

    // Records every notification it receives so tests can assert on
    // call counts and the last entity seen for each hook.
    class RecordingObserver : public IModelObserver<TestEntity>
    {
    public:
        void OnAdded(const TestEntity& entity) override
        {
            ++addedCount;
            lastAdded = entity.GetId();
        }

        void OnUpdated(const TestEntity& entity) override
        {
            ++updatedCount;
            lastUpdated = entity.GetId();
        }

        void OnRemoved(const TestEntity& entity) override
        {
            ++removedCount;
            lastRemoved = entity.GetId();
        }

        int addedCount = 0;
        int updatedCount = 0;
        int removedCount = 0;
        int lastAdded = -1;
        int lastUpdated = -1;
        int lastRemoved = -1;
    };

    class InMemoryModelTest : public ::testing::Test
    {
    protected:
        InMemoryModel<TestEntity> model;
    };
}

TEST_F(InMemoryModelTest, AddThenGetAllReturnsAddedEntity)
{
    model.Add(TestEntity(1, "first"));

    const std::vector<TestEntity> all = model.GetAll();

    ASSERT_EQ(all.size(), 1u);
    EXPECT_EQ(all[0].GetId(), 1);
    EXPECT_EQ(all[0].GetLabel(), "first");
}

TEST_F(InMemoryModelTest, GetByIdReturnsEntityWhenPresent)
{
    model.Add(TestEntity(1, "first"));

    const std::optional<TestEntity> found = model.GetById(1);

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetId(), 1);
    EXPECT_EQ(found->GetLabel(), "first");
}

TEST_F(InMemoryModelTest, GetByIdReturnsNulloptWhenAbsent)
{
    model.Add(TestEntity(1, "first"));

    const std::optional<TestEntity> found = model.GetById(999);

    EXPECT_FALSE(found.has_value());
}

TEST_F(InMemoryModelTest, FindReturnsOnlyMatchingEntities)
{
    model.Add(TestEntity(1, "alpha"));
    model.Add(TestEntity(2, "beta"));
    model.Add(TestEntity(3, "alpha"));

    const std::vector<TestEntity> matches =
        model.Find([](const TestEntity& entity) { return entity.GetLabel() == "alpha"; });

    ASSERT_EQ(matches.size(), 2u);
    for (const TestEntity& entity : matches)
    {
        EXPECT_EQ(entity.GetLabel(), "alpha");
    }
}

TEST_F(InMemoryModelTest, UpdateExistingEntitySucceedsAndReplacesValue)
{
    model.Add(TestEntity(1, "first"));

    const bool result = model.Update(TestEntity(1, "updated"));

    EXPECT_TRUE(result);
    const std::optional<TestEntity> found = model.GetById(1);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetLabel(), "updated");
}

TEST_F(InMemoryModelTest, UpdateNonExistentEntityFails)
{
    const bool result = model.Update(TestEntity(999, "does-not-exist"));

    EXPECT_FALSE(result);
}

TEST_F(InMemoryModelTest, RemoveExistingEntitySucceedsAndDropsIt)
{
    model.Add(TestEntity(1, "first"));

    const bool result = model.Remove(1);

    EXPECT_TRUE(result);
    EXPECT_FALSE(model.GetById(1).has_value());
    EXPECT_TRUE(model.GetAll().empty());
}

TEST_F(InMemoryModelTest, RemoveNonExistentEntityFails)
{
    const bool result = model.Remove(999);

    EXPECT_FALSE(result);
}

// The following tests deliberately go through IModel<T>& (not the concrete
// InMemoryModel<T>) to guard the Phase 1-A contract: Subscribe/Unsubscribe
// must be callable via the interface alone, since Controller/DataMonitor
// code is expected to hold only an IModel<T>* / IModel<T>&.

TEST_F(InMemoryModelTest, SubscribedObserverReceivesOnAddedThroughInterface)
{
    IModel<TestEntity>& modelInterface = model;
    RecordingObserver observer;
    modelInterface.Subscribe(&observer);

    modelInterface.Add(TestEntity(1, "first"));

    EXPECT_EQ(observer.addedCount, 1);
    EXPECT_EQ(observer.lastAdded, 1);
}

TEST_F(InMemoryModelTest, SubscribedObserverReceivesOnUpdatedThroughInterface)
{
    IModel<TestEntity>& modelInterface = model;
    modelInterface.Add(TestEntity(1, "first"));
    RecordingObserver observer;
    modelInterface.Subscribe(&observer);

    modelInterface.Update(TestEntity(1, "updated"));

    EXPECT_EQ(observer.updatedCount, 1);
    EXPECT_EQ(observer.lastUpdated, 1);
}

TEST_F(InMemoryModelTest, SubscribedObserverReceivesOnRemovedThroughInterface)
{
    IModel<TestEntity>& modelInterface = model;
    modelInterface.Add(TestEntity(1, "first"));
    RecordingObserver observer;
    modelInterface.Subscribe(&observer);

    modelInterface.Remove(1);

    EXPECT_EQ(observer.removedCount, 1);
    EXPECT_EQ(observer.lastRemoved, 1);
}

TEST_F(InMemoryModelTest, UnsubscribedObserverThroughInterfaceReceivesNoFurtherNotifications)
{
    IModel<TestEntity>& modelInterface = model;
    RecordingObserver observer;
    modelInterface.Subscribe(&observer);
    modelInterface.Add(TestEntity(1, "first"));
    ASSERT_EQ(observer.addedCount, 1);

    modelInterface.Unsubscribe(&observer);
    modelInterface.Add(TestEntity(2, "second"));
    modelInterface.Update(TestEntity(1, "updated"));
    modelInterface.Remove(1);

    EXPECT_EQ(observer.addedCount, 1);
    EXPECT_EQ(observer.updatedCount, 0);
    EXPECT_EQ(observer.removedCount, 0);
}
