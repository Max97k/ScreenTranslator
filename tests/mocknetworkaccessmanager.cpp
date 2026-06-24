#include "mocknetworkaccessmanager.h"

MockNetworkReply::MockNetworkReply(const QNetworkRequest &request, const QByteArray &data, QNetworkReply::NetworkError error, QObject *parent)
  : QNetworkReply(parent), data_(data), offset_(0), error_(error)
{
  setRequest(request);
  setOpenMode(QIODevice::ReadOnly);
  if (error == QNetworkReply::NoError) {
    setHeader(QNetworkRequest::ContentLengthHeader, data_.size());
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
  }

  QTimer::singleShot(0, this, &MockNetworkReply::finish);
}

qint64 MockNetworkReply::readData(char *data, qint64 maxlen)
{
  qint64 available = data_.size() - offset_;
  qint64 len = qMin(maxlen, available);
  if (len > 0) {
    memcpy(data, data_.constData() + offset_, len);
    offset_ += len;
  }
  return len;
}

qint64 MockNetworkReply::bytesAvailable() const
{
  return data_.size() - offset_ + QIODevice::bytesAvailable();
}

void MockNetworkReply::finish()
{
  if (error_ != QNetworkReply::NoError) {
    setError(error_, "Mock error");
  }
  setFinished(true);
  emit finished();
}


MockNetworkAccessManager::MockNetworkAccessManager(QObject *parent)
  : QNetworkAccessManager(parent), nextError_(QNetworkReply::NoError)
{
}

void MockNetworkAccessManager::setNextResponse(const QByteArray &data, QNetworkReply::NetworkError error)
{
  nextData_ = data;
  nextError_ = error;
}

QNetworkReply *MockNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
  Q_UNUSED(op);
  Q_UNUSED(outgoingData);
  auto reply = new MockNetworkReply(request, nextData_, nextError_, this);
  // Reset for next request if not sticky, or keep it. Let's reset.
  nextData_.clear();
  nextError_ = QNetworkReply::NoError;
  return reply;
}
