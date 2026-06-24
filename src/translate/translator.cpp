#include "translator.h"
#include "manager.h"
#include "settings.h"
#include "task.h"
#include "debug.h"
#include "languagecodes.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>

Translator::Translator(Manager &manager, const Settings &settings)
  : QObject(&manager)
  , manager_(manager)
  , settings_(settings)
  , networkManager_(new QNetworkAccessManager(this))
{
}

Translator::~Translator() = default;

void Translator::translate(const TaskPtr &task)
{
  if (!settings_.doTranslation || task->recognized.isEmpty()) {
    markTranslated(task, task->recognized);
    return;
  }

  translateWithGoogleCloud(task);
}

void Translator::updateSettings()
{
}

void Translator::finish(const TaskPtr &task)
{
}

QStringList Translator::availableTranslators(const QString &)
{
  return QStringList() << "Google Cloud API";
}

QStringList Translator::availableLanguageNames()
{
  return LanguageCodes::names();
}

void Translator::translateWithGoogleCloud(const TaskPtr &task)
{
  if (settings_.googleCloudApiKey.isEmpty()) {
    handleError(task, tr("Error: Google Cloud API Key is empty. Please configure it in Settings."));
    return;
  }

  QUrl url("https://translation.googleapis.com/language/translate/v2");
  QUrlQuery query;
  query.addQueryItem("key", settings_.googleCloudApiKey);
  url.setQuery(query);

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QJsonObject json;
  json["q"] = task->recognized;
  json["source"] = settings_.sourceLanguage;
  json["target"] = settings_.targetLanguage;
  json["format"] = "text";

  QByteArray data = QJsonDocument(json).toJson();
  QNetworkReply *reply = networkManager_->post(request, data);

  QTimer *timer = new QTimer(reply);
  timer->setSingleShot(true);
  connect(timer, &QTimer::timeout, reply, [reply]() {
    reply->abort();
  });
  timer->start(settings_.translationTimeout);

  connect(reply, &QNetworkReply::finished, this, [this, reply, task]() {
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
      handleError(task, tr("Network Error: ") + reply->errorString());
      return;
    }

    QByteArray response = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(response);
    QJsonObject root = doc.object();

    if (root.contains("error")) {
      handleError(task, tr("API Error: ") + root["error"].toObject()["message"].toString());
      return;
    }

    QJsonArray translations = root["data"].toObject()["translations"].toArray();
    if (translations.isEmpty()) {
      handleError(task, tr("Error: Empty translation result."));
      return;
    }

    QString translatedText = translations.first().toObject()["translatedText"].toString();
    markTranslated(task, translatedText);
  });
}

void Translator::markTranslated(const TaskPtr &task, const QString &translatedText)
{
  task->translated = translatedText;
  manager_.translated(task);
}

void Translator::handleError(const TaskPtr &task, const QString &errorMsg)
{
  task->translated = errorMsg;
  task->error = errorMsg;
  manager_.translated(task);
}
