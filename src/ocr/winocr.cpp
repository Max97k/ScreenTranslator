#include "winocr.h"
#include "debug.h"
#include "languagecodes.h"

#include <QBuffer>
#include <QImage>
#include <QPixmap>
#include <QStringList>
#include <QObject>

#if defined(Q_OS_WIN)
#include <windows.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Storage.Streams.h>
#include <MemoryBuffer.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Globalization;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Media::Ocr;
using namespace winrt::Windows::Storage::Streams;
#endif

WinOcr::WinOcr(const LanguageId &language, const QString &path)
{
  init(language);
}

WinOcr::~WinOcr() = default;

void WinOcr::init(const LanguageId &language)
{
#if defined(Q_OS_WIN)
  try {
    // Ensure COM is initialized on the background thread where this object is created.
    winrt::init_apartment();

    // Windows 11 OCR uses BCP-47 language tags.
    // E.g., "en-US", "ja", "ru". We map LanguageId to ISO 639-1 or whatever is available.
    QString langCode = LanguageCodes::iso639_1(language);
    Language lang(langCode.toStdWString());

    if (!OcrEngine::IsLanguageSupported(lang)) {
        error_ = QObject::tr("Language not supported by Windows OCR");
        valid_ = false;
        return;
    }

    engine_ = OcrEngine::TryCreateFromLanguage(lang);
    if (!engine_) {
        error_ = QObject::tr("Failed to create OCR engine for language");
        valid_ = false;
        return;
    }

    language_ = language;
    valid_ = true;
    LTRACE() << "Inited WinOcr api for" << language;
  } catch (const winrt::hresult_error& e) {
    error_ = QString::fromStdWString(std::wstring(e.message()));
    valid_ = false;
    LTRACE() << "Init WinOcr failed" << error_;
  }
#else
  error_ = QObject::tr("Windows OCR is only supported on Windows");
  valid_ = false;
#endif
}

const QString &WinOcr::error() const
{
  return error_;
}

QStringList WinOcr::availableLanguageNames(const QString &path)
{
  QStringList names;
#if defined(Q_OS_WIN)
  try {
    auto langs = OcrEngine::AvailableRecognizerLanguages();

    const auto allIds = LanguageCodes::allIds();
    struct LangMap {
        QString iso;
        QString name;
    };
    std::vector<LangMap> mapping;
    mapping.reserve(allIds.size());
    for (const auto& id : allIds) {
        mapping.push_back({LanguageCodes::iso639_1(id), LanguageCodes::name(id)});
    }

    for (auto const& lang : langs) {
        QString tag = QString::fromStdWString(std::wstring(lang.LanguageTag()));
        // Attempt to find ScreenTranslator internal language id for this tag
        // Map from iso to name
        // For simplicity, just add the translated name if found, else just add the tag
        bool found = false;
        for (const auto& item : mapping) {
            if (item.iso == tag || item.iso.startsWith(tag)) {
                names.append(item.name);
                found = true;
                break;
            }
        }
        if (!found) {
            names.append(tag);
        }
    }
  } catch (...) {
      // Ignore
  }
#endif
  names.removeDuplicates();
  return names;
}

QString WinOcr::recognize(const QPixmap &source)
{
  SOFT_ASSERT(valid_, return {});
  SOFT_ASSERT(!source.isNull(), return {});

  error_.clear();
  QString result;

#if defined(Q_OS_WIN)
  try {
    // Recognize is called from a background worker thread. Ensure COM is initialized for this thread.
    winrt::init_apartment();

    QImage image = source.toImage();
    if (image.format() != QImage::Format_RGBA8888) {
      image = image.convertToFormat(QImage::Format_RGBA8888);
    }

    // Create a SoftwareBitmap from the QImage data
    BitmapPixelFormat format = BitmapPixelFormat::Rgba8;
    BitmapAlphaMode alpha = BitmapAlphaMode::Premultiplied;
    SoftwareBitmap softwareBitmap(format, image.width(), image.height(), alpha);

    // Copy data to SoftwareBitmap
    BitmapBuffer buffer = softwareBitmap.LockBuffer(BitmapBufferAccessMode::Write);
    IMemoryBufferReference reference = buffer.CreateReference();

    // Using interop to get byte array
    auto interop = reference.as<::Windows::Foundation::IMemoryBufferByteAccess>();
    uint8_t* dataInBytes;
    uint32_t capacity;
    winrt::check_hresult(interop->GetBuffer(&dataInBytes, &capacity));

    // QImage RGBA8888 has 4 bytes per pixel.
    memcpy(dataInBytes, image.bits(), (std::min)(static_cast<size_t>(capacity), static_cast<size_t>(image.sizeInBytes())));

    reference.Close();
    buffer.Close();

    if (!engine_) {
       error_ = QObject::tr("OCR engine not initialized");
       return {};
    }

    OcrResult ocrResult = engine_.RecognizeAsync(softwareBitmap).get();

    for (auto const& line : ocrResult.Lines()) {
        if (!result.isEmpty()) result += "\n";
        result += QString::fromStdWString(std::wstring(line.Text()));
    }

  } catch (const winrt::hresult_error& e) {
    error_ = QString::fromStdWString(std::wstring(e.message()));
    LTRACE() << "WinOcr recognize error" << error_;
  } catch (...) {
    error_ = QObject::tr("Unknown error during WinOcr recognize");
    LTRACE() << "WinOcr recognize error unknown";
  }
#else
  error_ = QObject::tr("Windows OCR is only supported on Windows");
#endif

  if (result.isEmpty() && error_.isEmpty())
    error_ = QObject::tr("Failed to recognize text or no text selected");

  return result;
}

bool WinOcr::isValid() const
{
  return valid_;
}
