// Example/Model/AppendOnlyFilePersistenceAdapter.h contract test (PoC
// verification domain).
//
// docs/design/phase4.md item 3 / completion criterion 3 - "swapping the
// persistence adapter must not change IModel<T>/Controller/View's public
// interface" - and its test point - "existing Model tests must still pass
// with the persistence adapter swapped in (regression check)".
//
// This file is the actual proof: it re-runs the same Add/GetById/GetAll
// scenario as Tests/Example/SampleTest.cpp's
// InMemoryModelAddGetByIdGetAllIntegration case, but constructs
// InMemoryModel<Sample> with AppendOnlyFilePersistenceAdapter<Sample>
// injected instead of the default NullPersistenceAdapter<Sample>. Nothing
// about IModel<Sample>'s public surface changes; only construction differs.
//
// It additionally verifies the adapter's own contract: Save() (invoked by
// InMemoryModel<T> on every Add/Update) appends a new block to the backing
// file rather than overwriting it, and Load() never restores anything (see
// AppendOnlyFilePersistenceAdapter.h's class comment for why round-tripping
// is out of scope for this PoC).

#include <gtest/gtest.h>

#include "../../Example/Model/AppendOnlyFilePersistenceAdapter.h"
#include "../../Example/Model/Sample.h"
#include "../../Model/InMemoryModel.h"

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

namespace
{
    using ConsoleMVC::Example::Model::AppendOnlyFilePersistenceAdapter;
    using ConsoleMVC::Example::Model::Sample;
    using ConsoleMVC::Model::InMemoryModel;

    std::string SerializeSample(const Sample& sample)
    {
        std::ostringstream out;
        out << sample.GetId() << ',' << sample.GetName() << ',' << sample.GetStockQuantity();
        return out.str();
    }

    std::string ReadFile(const std::filesystem::path& path)
    {
        std::ifstream file(path);
        std::ostringstream contents;
        contents << file.rdbuf();
        return contents.str();
    }

    // Wraps a unique temp file path and deletes it on scope exit so tests
    // never leak files into the OS temp directory across runs.
    class ScopedTempFile
    {
    public:
        explicit ScopedTempFile(const std::string& fileName)
            : m_path(std::filesystem::temp_directory_path() / fileName)
        {
            std::filesystem::remove(m_path);
        }

        ~ScopedTempFile()
        {
            std::error_code ignored;
            std::filesystem::remove(m_path, ignored);
        }

        const std::filesystem::path& Path() const { return m_path; }

    private:
        std::filesystem::path m_path;
    };
}

TEST(AppendOnlyFilePersistenceAdapterTest, InMemoryModelCrudWorksUnchangedWithFileAdapterInjected)
{
    ScopedTempFile tempFile("ConsoleMVC_AppendOnlyFilePersistenceAdapterTest_Crud.txt");

    auto adapter = std::make_shared<AppendOnlyFilePersistenceAdapter<Sample>>(
        tempFile.Path().string(), SerializeSample);
    InMemoryModel<Sample> model(adapter);

    // Same scenario as SampleTest.cpp's
    // InMemoryModelAddGetByIdGetAllIntegration case - proves IModel<Sample>'s
    // public interface is unaffected by which IPersistenceAdapter<Sample> is
    // injected.
    model.Add(Sample(1, "Alpha", 1.0, 0.8, 5));
    model.Add(Sample(2, "Beta", 2.0, 0.7, 3));

    const std::optional<Sample> found = model.GetById(1);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->GetName(), "Alpha");
    EXPECT_EQ(found->GetStockQuantity(), 5);

    const std::vector<Sample> all = model.GetAll();
    EXPECT_EQ(all.size(), 2u);

    Sample updated(1, "Alpha", 1.0, 0.8, 5);
    ASSERT_TRUE(updated.TryIncreaseStock(2));
    ASSERT_TRUE(model.Update(updated));
    EXPECT_EQ(model.GetById(1)->GetStockQuantity(), 7);

    ASSERT_TRUE(model.Remove(2));
    EXPECT_FALSE(model.GetById(2).has_value());
}

TEST(AppendOnlyFilePersistenceAdapterTest, AddPersistsSerializedEntityToFile)
{
    ScopedTempFile tempFile("ConsoleMVC_AppendOnlyFilePersistenceAdapterTest_Add.txt");

    auto adapter = std::make_shared<AppendOnlyFilePersistenceAdapter<Sample>>(
        tempFile.Path().string(), SerializeSample);
    InMemoryModel<Sample> model(adapter);

    model.Add(Sample(1, "Alpha", 1.0, 0.8, 5));

    const std::string contents = ReadFile(tempFile.Path());
    EXPECT_NE(contents.find(SerializeSample(Sample(1, "Alpha", 1.0, 0.8, 5))), std::string::npos);
}

TEST(AppendOnlyFilePersistenceAdapterTest, EachSaveAppendsRatherThanOverwritingPreviousContent)
{
    ScopedTempFile tempFile("ConsoleMVC_AppendOnlyFilePersistenceAdapterTest_Append.txt");

    auto adapter = std::make_shared<AppendOnlyFilePersistenceAdapter<Sample>>(
        tempFile.Path().string(), SerializeSample);
    InMemoryModel<Sample> model(adapter);

    model.Add(Sample(1, "Alpha", 1.0, 0.8, 5));
    const std::string afterFirstSave = ReadFile(tempFile.Path());

    model.Add(Sample(2, "Beta", 2.0, 0.7, 3));
    const std::string afterSecondSave = ReadFile(tempFile.Path());

    // The first save's block is still present after the second save, and the
    // file has grown - proof that Save() appends rather than replaces.
    EXPECT_NE(afterSecondSave.find(afterFirstSave), std::string::npos);
    EXPECT_GT(afterSecondSave.size(), afterFirstSave.size());
    EXPECT_NE(afterSecondSave.find(SerializeSample(Sample(2, "Beta", 2.0, 0.7, 3))), std::string::npos);
}

TEST(AppendOnlyFilePersistenceAdapterTest, LoadNeverRestoresPreviouslySavedEntities)
{
    ScopedTempFile tempFile("ConsoleMVC_AppendOnlyFilePersistenceAdapterTest_Load.txt");

    {
        auto adapter = std::make_shared<AppendOnlyFilePersistenceAdapter<Sample>>(
            tempFile.Path().string(), SerializeSample);
        InMemoryModel<Sample> model(adapter);
        model.Add(Sample(1, "Alpha", 1.0, 0.8, 5));
    }

    // A new InMemoryModel<Sample> backed by the same file starts empty -
    // this adapter is a write-only journal, not a restorable snapshot (see
    // AppendOnlyFilePersistenceAdapter.h's class comment).
    auto reopenedAdapter = std::make_shared<AppendOnlyFilePersistenceAdapter<Sample>>(
        tempFile.Path().string(), SerializeSample);
    InMemoryModel<Sample> reopenedModel(reopenedAdapter);

    EXPECT_TRUE(reopenedModel.GetAll().empty());
    EXPECT_NE(ReadFile(tempFile.Path()).find(SerializeSample(Sample(1, "Alpha", 1.0, 0.8, 5))), std::string::npos);
}
