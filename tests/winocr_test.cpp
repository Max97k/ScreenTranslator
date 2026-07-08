#include <gtest/gtest.h>
#include "../src/ocr/winocr.h"

TEST(WinOcrTest, InitThrowsOnInvalidLanguage) {
#if defined(Q_OS_WIN)
  // WinRT Language constructor throws if language tag is malformed (e.g. contains invalid characters for a BCP-47 tag)
  // Passing a malformed BCP-47 language tag (e.g. containing spaces)
  // will cause the winrt::Windows::Globalization::Language constructor to throw a winrt::hresult_error.
  WinOcr ocr("invalid tag with spaces", "");

  EXPECT_FALSE(ocr.isValid());
  EXPECT_FALSE(ocr.error().isEmpty());
  EXPECT_NE(ocr.error(), "Language not supported by Windows OCR");
  EXPECT_NE(ocr.error(), "Failed to create OCR engine for language");
#else
  WinOcr ocr("en", "");
  EXPECT_FALSE(ocr.isValid());
  EXPECT_EQ(ocr.error(), "Windows OCR is only supported on Windows");
#endif
}
