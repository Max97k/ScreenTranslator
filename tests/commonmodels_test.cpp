#include <gtest/gtest.h>
#include "../src/commonmodels.h"
#include "../src/ocr/winocr.h"
#include "../src/translate/translator.h"
#include <QStringListModel>
#include <QStringList>

// Stubs for static methods used by CommonModels
QStringList WinOcr::availableLanguageNames(const QString &path) {
    return {"Zzz", "Aaa", "Mmm"};
}

QStringList Translator::availableTranslators(const QString &path) {
    return {"Yyy", "Bbb", "Nnn"};
}

QStringList Translator::availableLanguageNames() {
    return {"Xxx", "Ccc", "Ooo"};
}

TEST(CommonModelsTest, UpdatePopulatesAndSortsModels) {
    CommonModels models;

    EXPECT_EQ(models.sourceLanguageModel()->rowCount(), 0);
    EXPECT_EQ(models.targetLanguageModel()->rowCount(), 0);
    EXPECT_TRUE(models.translators().isEmpty());

    models.update("tessdata", "translators");

    // Verify source language model is populated and sorted
    ASSERT_EQ(models.sourceLanguageModel()->rowCount(), 3);
    EXPECT_EQ(models.sourceLanguageModel()->stringList().at(0), "Aaa");
    EXPECT_EQ(models.sourceLanguageModel()->stringList().at(1), "Mmm");
    EXPECT_EQ(models.sourceLanguageModel()->stringList().at(2), "Zzz");

    // Verify translators are populated and sorted
    ASSERT_EQ(models.translators().size(), 3);
    EXPECT_EQ(models.translators().at(0), "Bbb");
    EXPECT_EQ(models.translators().at(1), "Nnn");
    EXPECT_EQ(models.translators().at(2), "Yyy");

    // Verify target language model is populated and sorted
    ASSERT_EQ(models.targetLanguageModel()->rowCount(), 3);
    EXPECT_EQ(models.targetLanguageModel()->stringList().at(0), "Ccc");
    EXPECT_EQ(models.targetLanguageModel()->stringList().at(1), "Ooo");
    EXPECT_EQ(models.targetLanguageModel()->stringList().at(2), "Xxx");

    // Verify targetLanguageModel is not repopulated if it already has rows
    models.targetLanguageModel()->setStringList({"KeepMe"});
    models.update("tessdata", "translators");
    ASSERT_EQ(models.targetLanguageModel()->rowCount(), 1);
    EXPECT_EQ(models.targetLanguageModel()->stringList().at(0), "KeepMe");
}
