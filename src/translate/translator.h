#pragma once

#include "stfwd.h"

#include <QWidget>



class Translator : public QWidget
{
  Q_OBJECT
public:
  Translator(Manager &manager, const Settings &settings);
  ~Translator();

  void translate(const TaskPtr &task);
  void updateSettings();
  void finish(const TaskPtr &task);

  static QStringList availableTranslators(const QString &path);
  static QStringList availableLanguageNames();

protected:
  void timerEvent(QTimerEvent *event) override;

private:
  void processQueue();
  void markTranslated(const TaskPtr &task);

  Manager &manager_;
  const Settings &settings_;
  std::vector<TaskPtr> queue_;
};
