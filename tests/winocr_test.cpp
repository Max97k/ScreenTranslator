#include <gtest/gtest.h>
#include <QPixmap>
#include <QImage>
#include <QStringList>
#include <QObject>
#include "../src/ocr/winocr.h"
#include "../src/languagecodes.h"

// The test file is only compiled on Windows via tests.pro
#if defined(Q_OS_WIN)
TEST(WinOcrTest, HandlesHResultErrorLargeImage) {
    WinOcr ocr(LanguageId("eng"), "");
    if (!ocr.isValid()) {
        GTEST_SKIP() << "OCR not available";
    }

    // Feed an image that exceeds MaxImageDimension (which is typically 2600 to 4000)
    // to cause an ArgumentException / hresult_error in WinRT OcrEngine
    // A 5000x5000 image consumes ~100MB, safely triggering the exception without causing OOM.
    QImage image(5000, 5000, QImage::Format_RGBA8888);
    image.fill(Qt::white);
    QPixmap pixmap = QPixmap::fromImage(image);

    QString result = ocr.recognize(pixmap);

    EXPECT_TRUE(result.isEmpty());
    EXPECT_FALSE(ocr.error().isEmpty());
    EXPECT_NE(ocr.error(), QObject::tr("Unknown error during WinOcr recognize"));
    EXPECT_NE(ocr.error(), QObject::tr("Failed to recognize text or no text selected"));
}

TEST(WinOcrTest, SuccessfulRecognition) {
    WinOcr ocr(LanguageId("eng"), "");
    if (!ocr.isValid()) {
        GTEST_SKIP() << "OCR not available";
    }

    QImage image(10, 10, QImage::Format_RGBA8888);
    image.fill(Qt::white);
    QPixmap pixmap = QPixmap::fromImage(image);

    QString result = ocr.recognize(pixmap);
    // Result might be empty if the image is just white, but there shouldn't be an error
    EXPECT_TRUE(ocr.error().isEmpty() || ocr.error() == QObject::tr("Failed to recognize text or no text selected"));
}
#endif
