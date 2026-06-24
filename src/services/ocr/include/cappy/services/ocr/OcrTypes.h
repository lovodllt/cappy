#pragma once

#include <QList>
#include <QRect>
#include <QString>
#include <QMetaType>

namespace cappy::services::ocr {

enum class OcrProvider {
    Local,
    Cloud,
};

struct OcrSettings {
    QString preferredProvider = "local";
    QString localCommand = "tesseract";
    QString localLanguage = "eng+chi_sim";
    QString cloudEndpoint = "https://api.openai.com/v1/chat/completions";
    QString cloudModel = "gpt-4.1-mini";
    QString cloudApiKey;
    QString cloudPrompt;
    int cloudTimeoutSeconds = 60;
};

struct OcrTextRegion {
    QRect rect;
    QString text;
    int confidence = -1;
};

struct OcrResult {
    QString text;
    QList<OcrTextRegion> regions;
};

[[nodiscard]] inline OcrProvider ocrProviderFromSettingsValue(const QString& value) {
    return value.trimmed().compare("cloud", Qt::CaseInsensitive) == 0 ? OcrProvider::Cloud
                                                                      : OcrProvider::Local;
}

[[nodiscard]] inline QString ocrProviderToSettingsValue(OcrProvider provider) {
    return provider == OcrProvider::Cloud ? "cloud" : "local";
}

} // namespace cappy::services::ocr

Q_DECLARE_METATYPE(cappy::services::ocr::OcrTextRegion)
Q_DECLARE_METATYPE(cappy::services::ocr::OcrResult)
