#include <gtest/gtest.h>
#include "../src/languagecodes.h"

TEST(LanguageCodesTest, IdForNameReturnsCorrectIdForKnownName) {
  EXPECT_EQ(LanguageCodes::idForName("English"), "eng");
  EXPECT_EQ(LanguageCodes::idForName("French"), "fra");
  EXPECT_EQ(LanguageCodes::idForName("Chinese (Simplified)"), "chi_sim");
  EXPECT_EQ(LanguageCodes::idForName("Any"), "any");
}

TEST(LanguageCodesTest, IdForNameReturnsInputForUnknownName) {
  EXPECT_EQ(LanguageCodes::idForName("UnknownLanguage"), "UnknownLanguage");
  EXPECT_EQ(LanguageCodes::idForName(""), "");
}

TEST(LanguageCodesTest, IdForTesseractReturnsCorrectIdForKnownTesseractCode) {
  EXPECT_EQ(LanguageCodes::idForTesseract("eng"), "eng");
  EXPECT_EQ(LanguageCodes::idForTesseract("fra"), "fra");
  EXPECT_EQ(LanguageCodes::idForTesseract("chi_sim"), "chi_sim");
  // "smo" has empty tesseract code in the map, so it maps back if we search for "", but "" maps to many things or nothing specific in find_if. Let's pick standard ones.
}

TEST(LanguageCodesTest, IdForTesseractReturnsInputForUnknownTesseractCode) {
  EXPECT_EQ(LanguageCodes::idForTesseract("unknown_code"), "unknown_code");
}

TEST(LanguageCodesTest, Iso639_1ReturnsCorrectCodeForKnownId) {
  EXPECT_EQ(LanguageCodes::iso639_1("eng"), "en");
  EXPECT_EQ(LanguageCodes::iso639_1("fra"), "fr");
  EXPECT_EQ(LanguageCodes::iso639_1("chi_sim"), "zh-CN");
}

TEST(LanguageCodesTest, Iso639_1ReturnsIdForUnknownId) {
  EXPECT_EQ(LanguageCodes::iso639_1("unknown_id"), "unknown_id");
}

TEST(LanguageCodesTest, TesseractReturnsCorrectCodeForKnownId) {
  EXPECT_EQ(LanguageCodes::tesseract("eng"), "eng");
  EXPECT_EQ(LanguageCodes::tesseract("fra"), "fra");
  EXPECT_EQ(LanguageCodes::tesseract("chi_sim"), "chi_sim");
  EXPECT_EQ(LanguageCodes::tesseract("smo"), ""); // Samoan has empty tesseract
}

TEST(LanguageCodesTest, TesseractReturnsIdForUnknownId) {
  EXPECT_EQ(LanguageCodes::tesseract("unknown_id"), "unknown_id");
}

TEST(LanguageCodesTest, NameReturnsCorrectNameForKnownId) {
  EXPECT_EQ(LanguageCodes::name("eng"), "English");
  EXPECT_EQ(LanguageCodes::name("fra"), "French");
  EXPECT_EQ(LanguageCodes::name("chi_sim"), "Chinese (Simplified)");
}

TEST(LanguageCodesTest, NameReturnsIdForUnknownId) {
  EXPECT_EQ(LanguageCodes::name("unknown_id"), "unknown_id");
}

TEST(LanguageCodesTest, AllIdsReturnsNonEmptyVector) {
  std::vector<LanguageId> ids = LanguageCodes::allIds();
  EXPECT_FALSE(ids.empty());
  // Check that some known IDs are in the vector
  EXPECT_NE(std::find(ids.begin(), ids.end(), "eng"), ids.end());
  EXPECT_NE(std::find(ids.begin(), ids.end(), "fra"), ids.end());
  EXPECT_NE(std::find(ids.begin(), ids.end(), "chi_sim"), ids.end());
}

TEST(LanguageCodesTest, AnyLanguageIdReturnsAny) {
  EXPECT_EQ(LanguageCodes::anyLanguageId(), "any");
}
