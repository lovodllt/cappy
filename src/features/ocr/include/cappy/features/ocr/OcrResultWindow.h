#pragma once

#include <QDialog>
#include <QImage>
#include <QString>

#include "cappy/localization/Localization.h"
#include "cappy/services/ocr/OcrTypes.h"

class QComboBox;
class QFrame;
class QLabel;
class QListWidget;
class QPlainTextEdit;
class QPushButton;
class QScrollArea;
class QSplitter;

namespace cappy::services::ocr {
class OcrService;
}

namespace cappy::features::ocr {

class OcrResultWindow final : public QDialog {
    Q_OBJECT

public:
    explicit OcrResultWindow(
        const QImage& image,
        cappy::services::ocr::OcrSettings settings,
        cappy::localization::AppLanguage language,
        QString appearanceMode,
        QWidget* parent = nullptr,
        cappy::services::ocr::OcrResult initialResult = {},
        bool autoRun = true
    );

private:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void buildUi();
    void refreshTexts();
    void runRecognition();
    void setZoomFactor(double factor);
    void updatePreview();
    void fitPreview();
    void copyRecognizedText();
    void saveRecognizedText();
    void refreshRegionList();
    void setActiveRegion(int index, bool syncTextCursor);
    void setHoveredRegion(int index);
    int regionIndexForTextCursor() const;
    int regionIndexAtPreviewPosition(const QPoint& position) const;
    void updateTextRegionSelection();

    QImage image_;
    cappy::services::ocr::OcrResult currentResult_;
    cappy::services::ocr::OcrSettings settings_;
    cappy::localization::AppLanguage language_ = cappy::localization::AppLanguage::English;
    QString appearanceMode_;
    cappy::services::ocr::OcrService* ocrService_ = nullptr;
    QWidget* toolbar_ = nullptr;
    QLabel* titleLabel_ = nullptr;
    QLabel* imageMetaLabel_ = nullptr;
    QLabel* previewTitleLabel_ = nullptr;
    QLabel* resultTitleLabel_ = nullptr;
    QLabel* regionTitleLabel_ = nullptr;
    QLabel* fullTextTitleLabel_ = nullptr;
    QPushButton* zoomInButton_ = nullptr;
    QPushButton* zoomOutButton_ = nullptr;
    QPushButton* fitButton_ = nullptr;
    QPushButton* copyButton_ = nullptr;
    QPushButton* saveButton_ = nullptr;
    QComboBox* providerComboBox_ = nullptr;
    QPushButton* runButton_ = nullptr;
    QLabel* zoomLabel_ = nullptr;
    QLabel* previewLabel_ = nullptr;
    QScrollArea* previewScrollArea_ = nullptr;
    QSplitter* resultSplitter_ = nullptr;
    QListWidget* regionListWidget_ = nullptr;
    QPlainTextEdit* resultEdit_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    double zoomFactor_ = 1.0;
    int activeRegionIndex_ = -1;
    int hoveredRegionIndex_ = -1;
    bool suppressTextCursorSync_ = false;
    bool autoRun_ = true;
};

}  // namespace cappy::features::ocr
