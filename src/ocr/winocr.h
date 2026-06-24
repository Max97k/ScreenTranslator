#pragma once

#include "stfwd.h"

#include <QString>
#include <QStringList>

#if defined(Q_OS_WIN)
#include <winrt/Windows.Media.Ocr.h>
#endif

class QPixmap;

class WinOcr
{
public:
  WinOcr(const LanguageId& language, const QString& path);
  ~WinOcr();

  QString recognize(const QPixmap& source);
  bool isValid() const;
  const QString& error() const;

  static QStringList availableLanguageNames(const QString& path);

private:
  void init(const LanguageId& language);

  bool valid_ = false;
  QString error_;
  LanguageId language_;

#if defined(Q_OS_WIN)
  winrt::Windows::Media::Ocr::OcrEngine engine_{nullptr};
#endif
};
