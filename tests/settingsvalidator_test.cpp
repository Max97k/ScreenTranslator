#include <gtest/gtest.h>

#include "../src/settingsvalidator.h"
#include "../src/settings.h"
#include "../src/commonmodels.h"
#include <QStringList>

TEST(SettingsValidatorTest, CorrectReturnsFalseWhenDoTranslationIsFalse)
{
  SettingsValidator validator;
  Settings settings;
  settings.doTranslation = false;
  settings.translators.clear();

  CommonModels models;
  models.update("", ""); // populated

  bool changed = validator.correct(settings, models);

  EXPECT_FALSE(changed);
  EXPECT_TRUE(settings.translators.isEmpty());
}

TEST(SettingsValidatorTest, CorrectReturnsFalseWhenSettingsTranslatorsIsNotEmpty)
{
  SettingsValidator validator;
  Settings settings;
  settings.doTranslation = true;
  settings.translators = QStringList() << "SomeTranslator";

  CommonModels models;
  models.update("", ""); // populated

  bool changed = validator.correct(settings, models);

  EXPECT_FALSE(changed);
  EXPECT_EQ(settings.translators.size(), 1);
  EXPECT_EQ(settings.translators.first(), "SomeTranslator");
}

TEST(SettingsValidatorTest, CorrectReturnsFalseWhenModelsTranslatorsIsEmpty)
{
  SettingsValidator validator;
  Settings settings;
  settings.doTranslation = true;
  settings.translators.clear();

  CommonModels models;
  // Not calling update() means models.translators() is empty

  bool changed = validator.correct(settings, models);

  EXPECT_FALSE(changed);
  EXPECT_TRUE(settings.translators.isEmpty());
}

TEST(SettingsValidatorTest, CorrectReturnsTrueAndUpdatesSettingsTranslators)
{
  SettingsValidator validator;
  Settings settings;
  settings.doTranslation = true;
  settings.translators.clear();

  CommonModels models;
  models.update("", ""); // populated

  bool changed = validator.correct(settings, models);

  EXPECT_TRUE(changed);
  EXPECT_FALSE(settings.translators.isEmpty());

  // Verify it was copied correctly
  const QStringList& expectedTranslators = models.translators();
  EXPECT_EQ(settings.translators.size(), expectedTranslators.size());
  for (int i = 0; i < settings.translators.size(); ++i) {
    EXPECT_EQ(settings.translators.at(i), expectedTranslators.at(i));
  }
}
