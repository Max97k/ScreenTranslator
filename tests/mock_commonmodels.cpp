#include "../src/commonmodels.h"
#include <QStringListModel>

CommonModels::CommonModels()
  : sourceLanguageModel_(std::make_unique<QStringListModel>())
  , targetLanguageModel_(std::make_unique<QStringListModel>())
{
}

CommonModels::~CommonModels() = default;

void CommonModels::update(const QString &, const QString &)
{
  translators_ = QStringList() << "MockTranslator";
}

QStringListModel *CommonModels::sourceLanguageModel() const
{
  return sourceLanguageModel_.get();
}

QStringListModel *CommonModels::targetLanguageModel() const
{
  return targetLanguageModel_.get();
}

const QStringList &CommonModels::translators() const
{
  return translators_;
}
