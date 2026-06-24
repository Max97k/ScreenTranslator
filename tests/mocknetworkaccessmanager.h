#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

class MockNetworkReply : public QNetworkReply
{
  Q_OBJECT
public:
  MockNetworkReply(const QNetworkRequest &request, const QByteArray &data, QNetworkReply::NetworkError error = QNetworkReply::NoError, QObject *parent = nullptr);

  void abort() override {}
  qint64 readData(char *data, qint64 maxlen) override;
  qint64 bytesAvailable() const override;

private slots:
  void finish();

private:
  QByteArray data_;
  qint64 offset_;
  QNetworkReply::NetworkError error_;
};

class MockNetworkAccessManager : public QNetworkAccessManager
{
  Q_OBJECT
public:
  explicit MockNetworkAccessManager(QObject *parent = nullptr);

  void setNextResponse(const QByteArray &data, QNetworkReply::NetworkError error = QNetworkReply::NoError);

protected:
  QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData) override;

private:
  QByteArray nextData_;
  QNetworkReply::NetworkError nextError_;
};
