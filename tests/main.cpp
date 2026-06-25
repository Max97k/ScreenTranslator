#include <QApplication>
#include <QSettings>

#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
  qputenv("QT_QPA_PLATFORM", "offscreen");
  QApplication a(argc, argv);
  QCoreApplication::setOrganizationName("ScreenTranslator");
  QCoreApplication::setApplicationName("ScreenTranslatorTests");

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
