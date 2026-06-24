#pragma once

#include "stfwd.h"

#include <QDebug>
#include <QPixmap>
#include <future>

class Task
{
public:
  Task() = default;
  bool isNull() const { return captured.isNull() && !sourceLanguage.isEmpty(); }
  bool isValid() const { return error.isEmpty(); }

  Generation generation{};

  QPoint capturePoint;
  QPixmap captured;
  QString recognized;
  QString corrected;
  QString translated;

  bool useHunspell{false};

  LanguageId sourceLanguage;
  LanguageId targetLanguage;

  QStringList translators;
  QString usedTranslator;

  QString error;
  QStringList translatorErrors;

  std::shared_ptr<std::promise<void>> capturePromise{std::make_shared<std::promise<void>>()};
  std::shared_ptr<std::promise<void>> ocrPromise{std::make_shared<std::promise<void>>()};
  std::shared_ptr<std::promise<void>> correctPromise{std::make_shared<std::promise<void>>()};
  std::shared_ptr<std::promise<void>> translatePromise{std::make_shared<std::promise<void>>()};

  std::shared_ptr<std::future<void>> captureFuture{std::make_shared<std::future<void>>(capturePromise->get_future())};
  std::shared_ptr<std::future<void>> ocrFuture{std::make_shared<std::future<void>>(ocrPromise->get_future())};
  std::shared_ptr<std::future<void>> correctFuture{std::make_shared<std::future<void>>(correctPromise->get_future())};
  std::shared_ptr<std::future<void>> translateFuture{std::make_shared<std::future<void>>(translatePromise->get_future())};
};

using TaskPtr = std::shared_ptr<Task>;

Q_DECLARE_METATYPE(TaskPtr);

inline QDebug operator<<(QDebug debug, const TaskPtr &c)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << "Task(Gen=" << c->generation
                  << ", pix=" << c->captured.size() << ", rec=" << c->recognized
                  << ", cor=" << c->corrected << ", tr=" << c->translated
                  << ", lang=" << qPrintable(c->sourceLanguage) << '-'
                  << qPrintable(c->targetLanguage) << ", err=" << c->error
                  << ')';

  return debug;
}
