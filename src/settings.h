#pragma once

#include "stfwd.h"

#include <QColor>
#include <QDateTime>
#include <QStringList>

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <QSettings>

class QSettings;

enum class ResultMode { Widget, Tooltip };

struct Substitution {
  QString source;
  QString target;
};
using Substitutions = std::multimap<LanguageId, Substitution>;

enum class ProxyType { Disabled, System, Socks5, Http };

class Settings
{
public:
  Settings() = default;
  Settings(const Settings& other);
  Settings& operator=(const Settings& other);

  void save() const;
  void load();

  void saveLastUpdateCheck();

  bool isPortable() const;
  void setPortable(bool isPortable);

  QString captureHotkey{"Ctrl+Alt+Z"};
  QString repeatCaptureHotkey{"Ctrl+Alt+S"};
  QString showLastHotkey{"Ctrl+Alt+X"};
  QString clipboardHotkey{"Ctrl+Alt+C"};
  QString captureLockedHotkey{"Ctrl+Alt+Q"};

  bool showMessageOnStart{true};
  bool runAtSystemStart{false};

  ProxyType proxyType{ProxyType::System};
  QString proxyHostName;
  int proxyPort{8080};
  QString proxyUser;
  QString proxyPassword;
  bool proxySavePassword{false};

  int autoUpdateIntervalDays{0};
  QDateTime lastUpdateCheck;

  bool useHunspell{false};
  QString hunspellPath;
  Substitutions userSubstitutions;
  bool useUserSubstitutions{true};

  bool writeTrace{false};

  QString tessdataPath;
  QString sourceLanguage{"eng"};

  bool doTranslation{true};
  LanguageId targetLanguage{"rus"};
  std::chrono::seconds translationTimeout{15};
  QString translatorsPath;
  QStringList translators{"google.js"};
  QString googleCloudApiKey;

  ResultMode resultShowType{ResultMode::Widget};  // dialog
  QString fontFamily;
  int fontSize{11};
  QColor fontColor{Qt::black};
  QColor backgroundColor{Qt::lightGray};
  bool showRecognized{true};
  bool showCaptured{true};

private:
  QSettings& qsettings() const;

  void loadGui(QSettings& settings);
  void loadRecognition(QSettings& settings);
  void loadCorrection(QSettings& settings);
  void loadTranslation(QSettings& settings);
  void loadRepresentation(QSettings& settings);

  void saveGui(QSettings& settings) const;
  void saveRecognition(QSettings& settings) const;
  void saveCorrection(QSettings& settings) const;
  void saveTranslation(QSettings& settings) const;
  void saveRepresentation(QSettings& settings) const;

  bool isPortable_{false};
  mutable std::mutex mutex_;
  mutable std::unique_ptr<QSettings> qsettings_;
};
