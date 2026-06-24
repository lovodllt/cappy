#include "cappy/services/ocr/OcrService.h"

#include <algorithm>
#include <memory>

#include <QBuffer>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QStringConverter>
#include <QStringList>
#include <QTemporaryFile>
#include <QTextStream>
#include <QUrl>

namespace cappy::services::ocr {

namespace {

QString defaultCloudPrompt() {
    return QStringLiteral(
        "Recognize all visible text in the image. Return plain text only. Preserve line breaks.");
}

constexpr int kMinUsefulWordConfidence = 40;

bool isCjkCharacter(QChar character) {
    const char32_t codePoint = character.unicode();
    return (codePoint >= 0x3400 && codePoint <= 0x4DBF) ||
           (codePoint >= 0x4E00 && codePoint <= 0x9FFF) ||
           (codePoint >= 0xF900 && codePoint <= 0xFAFF);
}

void appendRecognizedToken(QString& line, const QString& token) {
    if (token.isEmpty()) {
        return;
    }

    if (line.isEmpty()) {
        line = token;
        return;
    }

    const QChar previous = line.back();
    const QChar next = token.front();
    if (!isCjkCharacter(previous) && !isCjkCharacter(next)) {
        line += QLatin1Char(' ');
    }
    line += token;
}

void flushMergedLineRegion(OcrResult& result, QString& currentLineText, QRect& currentLineRect,
                           int& currentLineConfidenceSum, int& currentLineConfidenceCount) {
    if (currentLineText.isEmpty()) {
        currentLineRect = {};
        currentLineConfidenceSum = 0;
        currentLineConfidenceCount = 0;
        return;
    }

    OcrTextRegion region;
    region.rect = currentLineRect;
    region.text = currentLineText;
    region.confidence =
        currentLineConfidenceCount > 0
            ? qRound(static_cast<double>(currentLineConfidenceSum) / currentLineConfidenceCount)
            : -1;
    result.regions.push_back(region);

    currentLineText.clear();
    currentLineRect = {};
    currentLineConfidenceSum = 0;
    currentLineConfidenceCount = 0;
}

OcrResult parseTsvOutput(const QString& tsv) {
    OcrResult result;

    QTextStream stream(const_cast<QString*>(&tsv), QIODevice::ReadOnly);
    stream.setEncoding(QStringConverter::Utf8);
    if (stream.atEnd()) {
        return result;
    }

    const QString headerLine = stream.readLine();
    const QStringList headers = headerLine.split('\t', Qt::KeepEmptyParts);
    QHash<QString, int> columnIndexByName;
    for (int index = 0; index < headers.size(); ++index) {
        columnIndexByName.insert(headers.at(index).trimmed(), index);
    }

    const int levelIndex = columnIndexByName.value(QStringLiteral("level"), -1);
    const int blockIndex = columnIndexByName.value(QStringLiteral("block_num"), -1);
    const int paragraphIndex = columnIndexByName.value(QStringLiteral("par_num"), -1);
    const int lineIndex = columnIndexByName.value(QStringLiteral("line_num"), -1);
    const int leftIndex = columnIndexByName.value(QStringLiteral("left"), -1);
    const int topIndex = columnIndexByName.value(QStringLiteral("top"), -1);
    const int widthIndex = columnIndexByName.value(QStringLiteral("width"), -1);
    const int heightIndex = columnIndexByName.value(QStringLiteral("height"), -1);
    const int confidenceIndex = columnIndexByName.value(QStringLiteral("conf"), -1);
    const int textIndex = columnIndexByName.value(QStringLiteral("text"), -1);
    if (levelIndex < 0 || blockIndex < 0 || paragraphIndex < 0 || lineIndex < 0 || leftIndex < 0 ||
        topIndex < 0 || widthIndex < 0 || heightIndex < 0 || confidenceIndex < 0 || textIndex < 0) {
        return result;
    }

    QString currentLineKey;
    QString currentLineText;
    QRect currentLineRect;
    int currentLineConfidenceSum = 0;
    int currentLineConfidenceCount = 0;
    QStringList lines;

    while (!stream.atEnd()) {
        const QString rowLine = stream.readLine();
        if (rowLine.isEmpty()) {
            continue;
        }

        const QStringList columns = rowLine.split('\t', Qt::KeepEmptyParts);
        if (columns.size() <=
            std::max({levelIndex, blockIndex, paragraphIndex, lineIndex, leftIndex, topIndex,
                      widthIndex, heightIndex, confidenceIndex, textIndex})) {
            continue;
        }

        bool ok = false;
        const int level = columns.at(levelIndex).toInt(&ok);
        if (!ok || level != 5) {
            continue;
        }

        const QString token = columns.at(textIndex).trimmed();
        if (token.isEmpty()) {
            continue;
        }

        const int left = columns.at(leftIndex).toInt(&ok);
        if (!ok) {
            continue;
        }
        const int top = columns.at(topIndex).toInt(&ok);
        if (!ok) {
            continue;
        }
        const int width = columns.at(widthIndex).toInt(&ok);
        if (!ok || width <= 0) {
            continue;
        }
        const int height = columns.at(heightIndex).toInt(&ok);
        if (!ok || height <= 0) {
            continue;
        }

        bool confidenceOk = false;
        const int confidence = qRound(columns.at(confidenceIndex).toDouble(&confidenceOk));
        if (confidenceOk && confidence >= 0 && confidence < kMinUsefulWordConfidence) {
            continue;
        }

        const QString lineKey = QStringLiteral("%1:%2:%3")
                                    .arg(columns.at(blockIndex))
                                    .arg(columns.at(paragraphIndex))
                                    .arg(columns.at(lineIndex));
        if (!currentLineKey.isEmpty() && currentLineKey != lineKey && !currentLineText.isEmpty()) {
            lines.push_back(currentLineText);
            flushMergedLineRegion(result, currentLineText, currentLineRect,
                                  currentLineConfidenceSum, currentLineConfidenceCount);
        }
        currentLineKey = lineKey;
        appendRecognizedToken(currentLineText, token);

        const QRect tokenRect(left, top, width, height);
        currentLineRect = currentLineRect.isNull() ? tokenRect : currentLineRect.united(tokenRect);
        if (confidenceOk && confidence >= 0) {
            currentLineConfidenceSum += confidence;
            currentLineConfidenceCount += 1;
        }
    }

    if (!currentLineText.isEmpty()) {
        lines.push_back(currentLineText);
        flushMergedLineRegion(result, currentLineText, currentLineRect, currentLineConfidenceSum,
                              currentLineConfidenceCount);
    }

    result.text = lines.join(QLatin1Char('\n')).trimmed();
    return result;
}

QString extractChatCompletionText(const QJsonDocument& document) {
    const QJsonObject root = document.object();
    const QJsonObject errorObject = root.value("error").toObject();
    if (!errorObject.isEmpty()) {
        return errorObject.value("message").toString();
    }

    const QJsonArray choices = root.value("choices").toArray();
    if (choices.isEmpty()) {
        return {};
    }

    const QJsonValue contentValue =
        choices.first().toObject().value("message").toObject().value("content");
    if (contentValue.isString()) {
        return contentValue.toString().trimmed();
    }
    if (contentValue.isArray()) {
        QStringList parts;
        for (const QJsonValue& partValue : contentValue.toArray()) {
            const QJsonObject partObject = partValue.toObject();
            const QString text = partObject.value("text").toString();
            if (!text.trimmed().isEmpty()) {
                parts.push_back(text.trimmed());
            }
        }
        return parts.join("\n").trimmed();
    }

    return {};
}

QString extractResponsesText(const QJsonDocument& document) {
    const QJsonObject root = document.object();
    const QJsonObject errorObject = root.value("error").toObject();
    if (!errorObject.isEmpty()) {
        return errorObject.value("message").toString();
    }

    const QString directText = root.value("output_text").toString().trimmed();
    if (!directText.isEmpty()) {
        return directText;
    }

    QStringList parts;
    const QJsonArray outputArray = root.value("output").toArray();
    for (const QJsonValue& itemValue : outputArray) {
        const QJsonArray contentArray = itemValue.toObject().value("content").toArray();
        for (const QJsonValue& contentValue : contentArray) {
            const QJsonObject contentObject = contentValue.toObject();
            const QString text = contentObject.value("text").toString().trimmed();
            if (!text.isEmpty()) {
                parts.push_back(text);
            }
        }
    }
    return parts.join("\n").trimmed();
}

} // namespace

OcrService::OcrService(QObject* parent)
    : QObject(parent), networkAccessManager_(std::make_unique<QNetworkAccessManager>(this)) {}

OcrService::~OcrService() {
    cancel();
}

bool OcrService::isBusy() const {
    return busy_;
}

void OcrService::recognize(const QImage& image, const OcrSettings& settings, OcrProvider provider) {
    if (busy_) {
        cancel();
    }

    if (image.isNull()) {
        finishWithError(QStringLiteral("Image is empty."));
        return;
    }

    setBusy(true);
    emit started();

    if (provider == OcrProvider::Cloud) {
        startCloudRecognition(image, settings);
        return;
    }

    startLocalRecognition(image, settings);
}

void OcrService::cancel() {
    if (process_ != nullptr) {
        process_->disconnect(this);
        process_->kill();
        process_->waitForFinished(1000);
    }
    if (activeReply_ != nullptr) {
        activeReply_->disconnect(this);
        activeReply_->abort();
        activeReply_->deleteLater();
        activeReply_ = nullptr;
    }
    clearActiveState();
}

void OcrService::setBusy(bool busy) {
    if (busy_ == busy) {
        return;
    }

    busy_ = busy;
    emit busyChanged(busy_);
}

void OcrService::startLocalRecognition(const QImage& image, const OcrSettings& settings) {
    const QString command = settings.localCommand.trimmed();
    if (command.isEmpty()) {
        finishWithError(QStringLiteral("Local OCR command is empty."));
        return;
    }

    temporaryImageFile_ = std::make_unique<QTemporaryFile>();
    temporaryImageFile_->setAutoRemove(false);
    temporaryImageFile_->setFileTemplate(QDir::tempPath() + "/cappy-ocr-XXXXXX.png");
    if (!temporaryImageFile_->open()) {
        finishWithError(QStringLiteral("Failed to create a temporary OCR image."));
        return;
    }
    image.save(temporaryImageFile_.get(), "PNG");
    temporaryImageFile_->close();

    process_ = std::make_unique<QProcess>(this);
    connect(
        process_.get(), &QProcess::errorOccurred, this,
        [this, command](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) {
                const QString resolved = QStandardPaths::findExecutable(command);
                finishWithError(
                    resolved.isEmpty()
                        ? QStringLiteral("Failed to start local OCR command: %1. Check that it is "
                                         "installed and available on PATH.")
                              .arg(command)
                        : QStringLiteral("Failed to start local OCR command: %1.").arg(command));
                return;
            }
            if (error == QProcess::Crashed) {
                finishWithError(QStringLiteral("Local OCR process crashed."));
                return;
            }
            finishWithError(QStringLiteral("Local OCR process failed."));
        });
    connect(process_.get(), &QProcess::finished, this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                const QString standardOutput =
                    QString::fromUtf8(process_->readAllStandardOutput()).trimmed();
                const QString standardError =
                    QString::fromUtf8(process_->readAllStandardError()).trimmed();
                if (exitStatus != QProcess::NormalExit || exitCode != 0) {
                    finishWithError(standardError.isEmpty()
                                        ? QStringLiteral("Local OCR exited with an error.")
                                        : standardError);
                    return;
                }
                finishWithResult(parseTsvOutput(standardOutput));
            });

    QStringList arguments;
    arguments << temporaryImageFile_->fileName() << "stdout";
    if (!settings.localLanguage.trimmed().isEmpty()) {
        arguments << "-l" << settings.localLanguage.trimmed();
    }
    arguments << "--psm" << "6" << "tsv";
    process_->start(command, arguments);
}

void OcrService::startCloudRecognition(const QImage& image, const OcrSettings& settings) {
    if (settings.cloudEndpoint.trimmed().isEmpty()) {
        finishWithError(QStringLiteral("Cloud OCR endpoint is empty."));
        return;
    }
    if (settings.cloudModel.trimmed().isEmpty()) {
        finishWithError(QStringLiteral("Cloud OCR model is empty."));
        return;
    }
    if (settings.cloudApiKey.trimmed().isEmpty()) {
        finishWithError(QStringLiteral("Cloud OCR API key is empty."));
        return;
    }

    QByteArray imageBytes;
    QBuffer buffer(&imageBytes);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    const QString imageDataUrl =
        QStringLiteral("data:image/png;base64,%1").arg(QString::fromLatin1(imageBytes.toBase64()));

    const QUrl endpoint(settings.cloudEndpoint.trimmed());
    const bool usesResponsesApi = endpoint.path().contains("/responses");

    QJsonObject payload;
    if (usesResponsesApi) {
        payload.insert("model", settings.cloudModel.trimmed());
        payload.insert(
            "input",
            QJsonArray{QJsonObject{
                {"role", "user"},
                {"content",
                 QJsonArray{QJsonObject{{"type", "input_text"},
                                        {"text", settings.cloudPrompt.trimmed().isEmpty()
                                                     ? defaultCloudPrompt()
                                                     : settings.cloudPrompt.trimmed()}},
                            QJsonObject{{"type", "input_image"}, {"image_url", imageDataUrl}}}}}});
    } else {
        payload.insert("model", settings.cloudModel.trimmed());
        payload.insert("temperature", 0.1);
        payload.insert(
            "messages",
            QJsonArray{
                QJsonObject{{"role", "system"},
                            {"content", settings.cloudPrompt.trimmed().isEmpty()
                                            ? defaultCloudPrompt()
                                            : settings.cloudPrompt.trimmed()}},
                QJsonObject{
                    {"role", "user"},
                    {"content",
                     QJsonArray{QJsonObject{{"type", "text"},
                                            {"text", "Recognize the text in this image."}},
                                QJsonObject{{"type", "image_url"},
                                            {"image_url", QJsonObject{{"url", imageDataUrl}}}}}}}});
    }

    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization",
                         QByteArray("Bearer ") + settings.cloudApiKey.trimmed().toUtf8());
    request.setTransferTimeout(std::max(5, settings.cloudTimeoutSeconds) * 1000);

    activeReply_ =
        networkAccessManager_->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(activeReply_, &QNetworkReply::finished, this, [this, usesResponsesApi]() {
        const QByteArray responseBytes = activeReply_->readAll();
        const QString networkError = activeReply_->error() == QNetworkReply::NoError
                                         ? QString{}
                                         : activeReply_->errorString();
        activeReply_->deleteLater();
        activeReply_ = nullptr;

        if (!networkError.isEmpty()) {
            finishWithError(networkError);
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(responseBytes);
        const QString text =
            usesResponsesApi ? extractResponsesText(document) : extractChatCompletionText(document);
        if (text.isEmpty()) {
            finishWithError(QStringLiteral("Cloud OCR returned an empty response."));
            return;
        }
        OcrResult result;
        result.text = text;
        finishWithResult(result);
    });
}

void OcrService::finishWithError(const QString& message) {
    clearActiveState();
    emit failed(message);
}

void OcrService::finishWithResult(const OcrResult& result) {
    clearActiveState();
    emit finished(result);
}

void OcrService::clearActiveState() {
    if (temporaryImageFile_ != nullptr) {
        QFile::remove(temporaryImageFile_->fileName());
        temporaryImageFile_.reset();
    }
    process_.reset();
    setBusy(false);
}

} // namespace cappy::services::ocr
