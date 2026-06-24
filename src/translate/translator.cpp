#include "translator.h"
#include "debug.h"
#include "languagecodes.h"
#include "manager.h"
#include "settings.h"
#include "task.h"
#include "widgetstate.h"

#include <QDir>


Translator::Translator(Manager &manager, const Settings &settings)
  : manager_(manager)
  , settings_(settings)
{
  setObjectName("Translator");

  new service::WidgetState(this);
}

Translator::~Translator() = default;

void Translator::translate(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );

  if (task->corrected.isEmpty()) {
    LTRACE() << "Corrected text is empty. Skipping translation";
    manager_.translated(task);
    return;
  }

  queue_.push_back(task);
  processQueue();
}

void Translator::updateSettings()
{
  queue_.clear();
}

void Translator::processQueue()
{
  if (queue_.empty())
    return;
}

void Translator::markTranslated(const TaskPtr &task)
{
  manager_.translated(task);
  queue_.erase(std::remove(queue_.begin(), queue_.end(), task), queue_.end());
}

void Translator::finish(const TaskPtr &task)
{
  markTranslated(task);
  processQueue();
}

QStringList Translator::availableTranslators(const QString &path)
{
  if (path.isEmpty())
    return {};

  QDir dir(path);
  if (!dir.exists())
    return {};

  const auto names = dir.entryList({"*.js"}, QDir::Files);
  return names;
}

QStringList Translator::availableLanguageNames()
{
  QStringList names;

  for (const auto &id : LanguageCodes::allIds()) {
    const auto iso = LanguageCodes::iso639_1(id);
    if (!iso.isEmpty())
      names.append(LanguageCodes::name(id));
  }

  return names;
}

void Translator::timerEvent(QTimerEvent * /*event*/)
{
  processQueue();
}
