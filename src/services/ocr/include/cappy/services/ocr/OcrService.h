#pragma once

#include <memory>

#include <QImage>
#include <QObject>

#include "cappy/services/ocr/OcrTypes.h"

class QNetworkAccessManager;
class QNetworkReply;
class QProcess;
class QTemporaryFile;

namespace cappy::services::ocr {

class OcrService final : public QObject {
    Q_OBJECT

  public:
    explicit OcrService(QObject* parent = nullptr);
    ~OcrService() override;

    [[nodiscard]] bool isBusy() const;
    void recognize(const QImage& image, const OcrSettings& settings, OcrProvider provider);
    void cancel();

  signals:
    void started();
    void finished(const cappy::services::ocr::OcrResult& result);
    void failed(const QString& message);
    void busyChanged(bool busy);

  private:
    void setBusy(bool busy);
    void startLocalRecognition(const QImage& image, const OcrSettings& settings);
    void startCloudRecognition(const QImage& image, const OcrSettings& settings);
    void finishWithError(const QString& message);
    void finishWithResult(const cappy::services::ocr::OcrResult& result);
    void clearActiveState();

    bool busy_ = false;
    std::unique_ptr<QProcess> process_;
    std::unique_ptr<QTemporaryFile> temporaryImageFile_;
    std::unique_ptr<QNetworkAccessManager> networkAccessManager_;
    QNetworkReply* activeReply_ = nullptr;
};

} // namespace cappy::services::ocr
