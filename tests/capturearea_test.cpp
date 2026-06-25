#include <gtest/gtest.h>

#include "../src/capture/capturearea.h"
#include "../src/settings.h"
#include "../src/task.h"

#include <QPixmap>
#include <QRect>
#include <QStringList>

class CaptureAreaTest : public ::testing::Test {
protected:
    Settings createSettings(bool doTranslation, const QString& sourceLang, const QString& targetLang, const QStringList& translators) {
        Settings settings;
        settings.doTranslation = doTranslation;
        settings.sourceLanguage = sourceLang;
        settings.targetLanguage = targetLang;
        settings.translators = translators;
        settings.useHunspell = true;
        return settings;
    }

    // Helper to create a non-null QPixmap
    QPixmap createValidPixmap() {
        QPixmap pixmap(10, 10);
        pixmap.fill(Qt::black);
        return pixmap;
    }
};

TEST_F(CaptureAreaTest, InvalidPixmap) {
    Settings settings = createSettings(true, "eng", "rus", {"google.js"});
    CaptureArea area(QRect(0, 0, 100, 100), settings);

    QPixmap invalidPixmap; // isNull() == true
    TaskPtr task = area.task(invalidPixmap, QPoint(0, 0));

    EXPECT_EQ(task, nullptr);
}

TEST_F(CaptureAreaTest, InvalidArea) {
    Settings settings = createSettings(true, "eng", "rus", {"google.js"});
    CaptureArea area(QRect(0, 0, 2, 2), settings); // isValid() == false (width or height < 3)

    QPixmap validPixmap = createValidPixmap();
    TaskPtr task = area.task(validPixmap, QPoint(0, 0));

    EXPECT_EQ(task, nullptr);
}

TEST_F(CaptureAreaTest, ValidTask) {
    Settings settings = createSettings(true, "eng", "rus", {"google.js"});
    CaptureArea area(QRect(10, 10, 50, 50), settings);
    area.setGeneration(42);

    QPixmap validPixmap = createValidPixmap();
    QPoint offset(5, 5);
    TaskPtr task = area.task(validPixmap, offset);

    ASSERT_NE(task, nullptr);
    EXPECT_EQ(task->generation, 42u);
    EXPECT_EQ(task->useHunspell, true);
    // Note: The copy operates on the size of rect_, but here we just check if it's generated
    EXPECT_EQ(task->capturePoint, offset + QPoint(10, 10));
    EXPECT_EQ(task->sourceLanguage, "eng");
    EXPECT_EQ(task->targetLanguage, "rus");
    EXPECT_EQ(task->translators.size(), 1);
    EXPECT_EQ(task->translators.first(), "google.js");
    EXPECT_TRUE(task->error.isEmpty());
}

TEST_F(CaptureAreaTest, MissingSourceLanguage) {
    Settings settings = createSettings(true, "", "rus", {"google.js"});
    CaptureArea area(QRect(0, 0, 100, 100), settings);

    QPixmap validPixmap = createValidPixmap();
    TaskPtr task = area.task(validPixmap, QPoint(0, 0));

    ASSERT_NE(task, nullptr);
    EXPECT_TRUE(task->error.contains("No source language set"));
}

TEST_F(CaptureAreaTest, MissingTargetLanguage) {
    Settings settings = createSettings(true, "eng", "", {"google.js"});
    CaptureArea area(QRect(0, 0, 100, 100), settings);

    QPixmap validPixmap = createValidPixmap();
    TaskPtr task = area.task(validPixmap, QPoint(0, 0));

    ASSERT_NE(task, nullptr);
    EXPECT_TRUE(task->error.contains("No target language set"));
}

TEST_F(CaptureAreaTest, BothMissingLanguages) {
    Settings settings = createSettings(true, "", "", {"google.js"});
    CaptureArea area(QRect(0, 0, 100, 100), settings);

    QPixmap validPixmap = createValidPixmap();
    TaskPtr task = area.task(validPixmap, QPoint(0, 0));

    ASSERT_NE(task, nullptr);
    EXPECT_TRUE(task->error.contains("No source language set"));
    EXPECT_TRUE(task->error.contains("No target language set"));
}

TEST_F(CaptureAreaTest, NoTranslationNoTargetLanguageError) {
    // If doTranslation is false, it shouldn't care about missing target language
    Settings settings = createSettings(false, "eng", "", {"google.js"});
    CaptureArea area(QRect(0, 0, 100, 100), settings);

    QPixmap validPixmap = createValidPixmap();
    TaskPtr task = area.task(validPixmap, QPoint(0, 0));

    ASSERT_NE(task, nullptr);
    EXPECT_TRUE(task->error.isEmpty());
}
