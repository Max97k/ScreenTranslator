#include <gtest/gtest.h>
#include "../src/settingsvalidator.h"
#include "../src/settings.h"
#include "../src/commonmodels.h"
#include <QStringListModel>

static QStringList g_mockTranslators;

CommonModels::CommonModels() : sourceLanguageModel_(std::make_unique<QStringListModel>()) {}
CommonModels::~CommonModels() = default;

void CommonModels::update(const QString& tessdataPath, const QString& translatorPath) {
    Q_UNUSED(tessdataPath);
    Q_UNUSED(translatorPath);
}

QStringListModel* CommonModels::sourceLanguageModel() const {
  return sourceLanguageModel_.get();
}

QStringListModel* CommonModels::targetLanguageModel() const {
  return nullptr; // not used by validator
}

const QStringList& CommonModels::translators() const {
  return g_mockTranslators;
}

class SettingsValidatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        g_mockTranslators.clear();
        models.sourceLanguageModel()->setStringList(QStringList() << "eng");
        g_mockTranslators = QStringList() << "Google Cloud API";

        settings.sourceLanguage = "eng";
        settings.doTranslation = true;
        settings.translators = QStringList() << "Google Cloud API";
        settings.targetLanguage = "rus";
    }

    void TearDown() override {
        g_mockTranslators.clear();
    }

    SettingsValidator validator;
    Settings settings;
    CommonModels models;
};

TEST_F(SettingsValidatorTest, Check_NoErrors) {
    auto result = validator.check(settings, models);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(SettingsValidatorTest, Check_NoSourceInstalled) {
    models.sourceLanguageModel()->setStringList(QStringList()); // Empty
    auto result = validator.check(settings, models);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], SettingsValidator::Error::NoSourceInstalled);
}

TEST_F(SettingsValidatorTest, Check_NoSourceSet) {
    settings.sourceLanguage = "";
    auto result = validator.check(settings, models);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], SettingsValidator::Error::NoSourceSet);
}

TEST_F(SettingsValidatorTest, Check_NoTranslatorInstalled) {
    g_mockTranslators.clear();
    auto result = validator.check(settings, models);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], SettingsValidator::Error::NoTranslatorInstalled);
}

TEST_F(SettingsValidatorTest, Check_NoTranslatorSet) {
    settings.translators.clear();
    auto result = validator.check(settings, models);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], SettingsValidator::Error::NoTranslatorSet);
}

TEST_F(SettingsValidatorTest, Check_NoTargetSet) {
    settings.targetLanguage = "";
    auto result = validator.check(settings, models);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], SettingsValidator::Error::NoTargetSet);
}

TEST_F(SettingsValidatorTest, Check_MultipleErrors) {
    models.sourceLanguageModel()->setStringList(QStringList());
    settings.sourceLanguage = "";
    g_mockTranslators.clear();
    settings.translators.clear();
    settings.targetLanguage = "";

    auto result = validator.check(settings, models);
    ASSERT_EQ(result.size(), 5);
    EXPECT_EQ(result[0], SettingsValidator::Error::NoSourceInstalled);
    EXPECT_EQ(result[1], SettingsValidator::Error::NoSourceSet);
    EXPECT_EQ(result[2], SettingsValidator::Error::NoTranslatorInstalled);
    EXPECT_EQ(result[3], SettingsValidator::Error::NoTranslatorSet);
    EXPECT_EQ(result[4], SettingsValidator::Error::NoTargetSet);
}

TEST_F(SettingsValidatorTest, Check_TranslationDisabled_IgnoresTranslationErrors) {
    settings.doTranslation = false;
    settings.translators.clear();
    settings.targetLanguage = "";
    g_mockTranslators.clear();

    auto result = validator.check(settings, models);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(SettingsValidatorTest, Correct_NoChanges) {
    bool changed = validator.correct(settings, models);
    EXPECT_FALSE(changed);
    ASSERT_EQ(settings.translators.size(), 1);
    EXPECT_EQ(settings.translators[0], "Google Cloud API");
}

TEST_F(SettingsValidatorTest, Correct_AutoFillTranslators) {
    settings.translators.clear(); // Empty translators, but translation is enabled
    bool changed = validator.correct(settings, models);
    EXPECT_TRUE(changed);
    ASSERT_EQ(settings.translators.size(), 1);
    EXPECT_EQ(settings.translators[0], "Google Cloud API");
}

TEST_F(SettingsValidatorTest, Correct_AutoFillTranslators_IgnoresIfNoModelsAvailable) {
    settings.translators.clear();
    g_mockTranslators.clear(); // No translators available

    bool changed = validator.correct(settings, models);
    EXPECT_FALSE(changed);
    EXPECT_TRUE(settings.translators.isEmpty());
}

TEST_F(SettingsValidatorTest, Correct_AutoFillTranslators_IgnoresIfTranslationDisabled) {
    settings.doTranslation = false;
    settings.translators.clear();

    bool changed = validator.correct(settings, models);
    EXPECT_FALSE(changed);
    EXPECT_TRUE(settings.translators.isEmpty());
}

TEST_F(SettingsValidatorTest, ToString_ReturnsNonEmptyStrings) {
    EXPECT_FALSE(validator.toString(SettingsValidator::Error::NoSourceInstalled).isEmpty());
    EXPECT_FALSE(validator.toString(SettingsValidator::Error::NoSourceSet).isEmpty());
    EXPECT_FALSE(validator.toString(SettingsValidator::Error::NoTranslatorInstalled).isEmpty());
    EXPECT_FALSE(validator.toString(SettingsValidator::Error::NoTranslatorSet).isEmpty());
    EXPECT_FALSE(validator.toString(SettingsValidator::Error::NoTargetSet).isEmpty());
}
