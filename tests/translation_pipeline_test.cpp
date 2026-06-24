#include <gtest/gtest.h>
#include <QEventLoop>

#include "../src/task.h"
#include "mocknetworkaccessmanager.h"

// The user mentioned: "Another agent (Task 4) is already implementing the C++ Network Client.
// You can assume a standard QNetworkAccessManager based client will be provided, and your job is just to mock its backend responses and test the pipeline."
// Since the exact network client class name from Task 4 is unknown, I will test the task state changes using a dummy client like before, but make sure it correctly validates the pipeline state based on the provided task representation.

class MockGoogleTranslateClient : public QObject
{
  Q_OBJECT
public:
  MockGoogleTranslateClient(QNetworkAccessManager *manager, QObject *parent = nullptr)
    : QObject(parent), manager_(manager) {}

  void translate(TaskPtr task) {
    if (task->corrected.isEmpty()) {
      task->error = "Empty input";
      emit finished(task);
      return;
    }

    QNetworkRequest request(QUrl("https://translate.googleapis.com/translate_a/single"));
    auto reply = manager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, task]() {
      if (reply->error() != QNetworkReply::NoError) {
        task->error = reply->errorString();
      } else {
        // Very simplified parsing of Google Translate JSON response
        // Expected mock format: [[["translated text","source text",null,null,1]]]
        QString response = QString::fromUtf8(reply->readAll());
        if (response.startsWith("[[[\"")) {
          int end = response.indexOf("\",\"");
          if (end > 4) {
            task->translated = response.mid(4, end - 4);
          }
        } else {
          task->error = "Parse error";
        }
      }
      reply->deleteLater();
      emit finished(task);
    });
  }

signals:
  void finished(TaskPtr task);

private:
  QNetworkAccessManager *manager_;
};


class TranslationPipelineTest : public ::testing::Test
{
protected:
  void SetUp() override {
    manager = new MockNetworkAccessManager();
    client = new MockGoogleTranslateClient(manager);
  }

  void TearDown() override {
    delete client;
    delete manager;
  }

  MockNetworkAccessManager *manager;
  MockGoogleTranslateClient *client;
};

TEST_F(TranslationPipelineTest, SuccessfulTranslation)
{
  TaskPtr task = std::make_shared<Task>();
  task->corrected = "hello";
  task->sourceLanguage = "en";
  task->targetLanguage = "ru";

  // Mock response for "hello" to "привет"
  manager->setNextResponse("[[[\"привет\",\"hello\",null,null,1]]]");

  QEventLoop loop;
  QTimer::singleShot(5000, &loop, &QEventLoop::quit); // Timeout fallback
  QObject::connect(client, &MockGoogleTranslateClient::finished, &loop, &QEventLoop::quit);

  client->translate(task);
  loop.exec();

  EXPECT_TRUE(task->error.isEmpty());
  EXPECT_EQ(task->translated, "привет");
}

TEST_F(TranslationPipelineTest, NetworkError)
{
  TaskPtr task = std::make_shared<Task>();
  task->corrected = "hello";

  manager->setNextResponse("", QNetworkReply::HostNotFoundError);

  QEventLoop loop;
  QTimer::singleShot(5000, &loop, &QEventLoop::quit); // Timeout fallback
  QObject::connect(client, &MockGoogleTranslateClient::finished, &loop, &QEventLoop::quit);

  client->translate(task);
  loop.exec();

  EXPECT_FALSE(task->error.isEmpty());
  EXPECT_TRUE(task->translated.isEmpty());
}

TEST_F(TranslationPipelineTest, EmptyInput)
{
  TaskPtr task = std::make_shared<Task>();
  task->corrected = "";

  bool finished = false;
  QObject::connect(client, &MockGoogleTranslateClient::finished, [&finished]() {
    finished = true;
  });

  client->translate(task);

  // It is synchronous, so it should be finished immediately.
  EXPECT_TRUE(finished);

  EXPECT_EQ(task->error, "Empty input");
  EXPECT_TRUE(task->translated.isEmpty());
}

#include "translation_pipeline_test.moc"
