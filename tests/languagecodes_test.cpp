#include <gtest/gtest.h>
#include "languagecodes.h"

TEST(LanguageCodesTest, Iso639_1_ValidIds)
{
  EXPECT_EQ(LanguageCodes::iso639_1("afr"), "af");
  EXPECT_EQ(LanguageCodes::iso639_1("sqi"), "sq");
  EXPECT_EQ(LanguageCodes::iso639_1("eng"), "en");
}

TEST(LanguageCodesTest, Iso639_1_UnknownId)
{
  EXPECT_EQ(LanguageCodes::iso639_1("unknown"), "unknown");
}

TEST(LanguageCodesTest, Tesseract_ValidIds)
{
  EXPECT_EQ(LanguageCodes::tesseract("afr"), "afr");
  EXPECT_EQ(LanguageCodes::tesseract("eng"), "eng");
}

TEST(LanguageCodesTest, Tesseract_UnknownId)
{
  EXPECT_EQ(LanguageCodes::tesseract("unknown"), "unknown");
}

TEST(LanguageCodesTest, Name_ValidIds)
{
  // When testing without Qt translations loaded, it returns the string key.
  EXPECT_EQ(LanguageCodes::name("afr"), "Afrikaans");
  EXPECT_EQ(LanguageCodes::name("eng"), "English");
}

TEST(LanguageCodesTest, Name_UnknownId)
{
  EXPECT_EQ(LanguageCodes::name("unknown"), "unknown");
}
