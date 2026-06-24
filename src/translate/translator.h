#pragma once

#include "stfwd.h"
#include <QObject>
#include <QNetworkAccessManager>

class Translator : public QObject
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

private:
  void translateWithGoogleCloud(const TaskPtr &task);
  void markTranslated(const TaskPtr &task, const QString &translatedText);
  void handleError(const TaskPtr &task, const QString &errorMsg);

  Manager &manager_;
  const Settings &settings_;
  QNetworkAccessManager *networkManager_;
};
