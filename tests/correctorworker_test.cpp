#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QTextCodec>

#include "../src/correct/correctorworker.h"
#include "../src/task.h"
#include "../src/languagecodes.h"

TEST(CorrectorWorkerTest, EmptyTask) {
  qRegisterMetaType<TaskPtr>("TaskPtr");
  CorrectorWorker worker;
  TaskPtr emptyTask; // null
  worker.handle(emptyTask);
  SUCCEED();
}

TEST(CorrectorWorkerTest, InvalidTask) {
  qRegisterMetaType<TaskPtr>("TaskPtr");
  CorrectorWorker worker;
  TaskPtr task = std::make_shared<Task>();
  task->error = "Some error"; // making it invalid
  worker.handle(task);
  SUCCEED();
}

TEST(CorrectorWorkerTest, EmptyHunspellDir) {
  qRegisterMetaType<TaskPtr>("TaskPtr");
  CorrectorWorker worker;
  TaskPtr task = std::make_shared<Task>();
  task->corrected = "helo";
  task->sourceLanguage = "eng";
  worker.handle(task);
  SUCCEED();
}

TEST(CorrectorWorkerTest, InvalidHunspellEngine) {
  qRegisterMetaType<TaskPtr>("TaskPtr");
  CorrectorWorker worker;
  worker.reset("invalid_dir");

  TaskPtr task = std::make_shared<Task>();
  task->corrected = "helo";
  task->sourceLanguage = "eng";

  QSignalSpy spy(&worker, &CorrectorWorker::finished);
  worker.handle(task);

  ASSERT_EQ(spy.count(), 1);
  TaskPtr result = spy.takeFirst().at(0).value<TaskPtr>();

  EXPECT_EQ(result->corrected, QString("helo"));
}

TEST(CorrectorWorkerTest, SuccessfulCorrection) {
  qRegisterMetaType<TaskPtr>("TaskPtr");
  CorrectorWorker worker;

  // Create a dictionary on the fly to avoid path issues
  QString tempPath = QDir::tempPath() + "/hunspell_test_dir";
  QDir().mkpath(tempPath + "/en");
  QFile aff(tempPath + "/en/en.aff");
  if (aff.open(QIODevice::WriteOnly)) {
      aff.write("SET UTF-8\n");
      aff.close();
  }
  QFile dic(tempPath + "/en/en.dic");
  if (dic.open(QIODevice::WriteOnly)) {
      dic.write("1\nhello\n");
      dic.close();
  }

  worker.reset(tempPath);

  TaskPtr task = std::make_shared<Task>();
  task->corrected = "helo";
  task->sourceLanguage = "eng";

  QSignalSpy spy(&worker, &CorrectorWorker::finished);
  worker.handle(task);

  ASSERT_EQ(spy.count(), 1);
  TaskPtr result = spy.takeFirst().at(0).value<TaskPtr>();

  EXPECT_EQ(result->corrected.toStdString(), "hello");

  QDir(tempPath).removeRecursively();
}

TEST(CorrectorWorkerTest, RemoveUnusedGenerations) {
  qRegisterMetaType<TaskPtr>("TaskPtr");
  CorrectorWorker worker;

  QString tempPath = QDir::tempPath() + "/hunspell_test_dir";
  QDir().mkpath(tempPath + "/en");
  QFile aff(tempPath + "/en/en.aff");
  if (aff.open(QIODevice::WriteOnly)) {
      aff.write("SET UTF-8\n");
      aff.close();
  }
  QFile dic(tempPath + "/en/en.dic");
  if (dic.open(QIODevice::WriteOnly)) {
      dic.write("1\nhello\n");
      dic.close();
  }

  worker.reset(tempPath);

  for(int i = 1; i <= 15; ++i) {
    TaskPtr task = std::make_shared<Task>();
    task->corrected = "helo";
    task->sourceLanguage = "eng";
    task->generation = i;

    QSignalSpy spy(&worker, &CorrectorWorker::finished);
    worker.handle(task);

    ASSERT_EQ(spy.count(), 1);
    TaskPtr result = spy.takeFirst().at(0).value<TaskPtr>();
    EXPECT_EQ(result->corrected.toStdString(), "hello");
  }

  QDir().mkpath(tempPath + "/fr");
  QFile file(tempPath + "/fr/fr.aff");
  if(file.open(QIODevice::WriteOnly)) { file.write("SET UTF-8\n"); file.close(); }
  QFile file2(tempPath + "/fr/fr.dic");
  if(file2.open(QIODevice::WriteOnly)) { file2.write("1\nbonjour\n"); file2.close(); }

  for(int i = 16; i <= 30; ++i) {
    TaskPtr task = std::make_shared<Task>();
    task->corrected = "bonjor"; // length 6, 6*0.2=1 distance 1.
    task->sourceLanguage = "fra";
    task->generation = i;

    QSignalSpy spy(&worker, &CorrectorWorker::finished);
    worker.handle(task);

    ASSERT_EQ(spy.count(), 1);
    TaskPtr result = spy.takeFirst().at(0).value<TaskPtr>();
    EXPECT_EQ(result->corrected.toStdString(), "bonjour");
  }

  QDir(tempPath).removeRecursively();
}
