#include "../src/ocr/winocr.h"
#include "../src/translate/translator.h"

// Stubs for static methods used by CommonModels
QStringList WinOcr::availableLanguageNames(const QString &path) {
    Q_UNUSED(path);
    return {"Zzz", "Aaa", "Mmm"};
}

QStringList Translator::availableTranslators(const QString &path) {
    Q_UNUSED(path);
    return {"Yyy", "Bbb", "Nnn"};
}

QStringList Translator::availableLanguageNames() {
    return {"Xxx", "Ccc", "Ooo"};
}
