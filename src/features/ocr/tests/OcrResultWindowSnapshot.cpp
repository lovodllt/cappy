#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QTextCursor>
#include <QTimer>

#include "cappy/features/ocr/OcrResultWindow.h"

namespace {

QImage buildPreviewImage() {
    QImage image(1280, 760, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(236, 240, 244));

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(QRect(72, 84, 1136, 592), QColor(249, 251, 253));
    painter.setPen(QPen(QColor(199, 209, 219), 2));
    painter.drawRect(QRect(72, 84, 1136, 592));

    painter.setPen(QColor(48, 58, 68));
    QFont titleFont = painter.font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(QRect(128, 132, 900, 42), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Invoice #A-240624"));

    QFont bodyFont = painter.font();
    bodyFont.setPointSize(18);
    bodyFont.setBold(false);
    painter.setFont(bodyFont);
    painter.drawText(QRect(128, 206, 700, 34), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Customer  Sunny Capturer Studio"));
    painter.drawText(QRect(128, 260, 520, 34), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Date  2026-06-24"));
    painter.drawText(QRect(128, 314, 600, 34), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Amount  CNY 1299.00"));
    painter.drawText(QRect(128, 420, 880, 34), Qt::AlignLeft | Qt::AlignVCenter,
                     QStringLiteral("Please review the recognized text on the right."));
    painter.end();
    return image;
}

cappy::services::ocr::OcrResult buildMockResult() {
    using cappy::services::ocr::OcrResult;
    using cappy::services::ocr::OcrTextRegion;

    OcrResult result;
    result.text = QStringLiteral("Invoice #A-240624\n"
                                 "Customer  Sunny Capturer Studio\n"
                                 "Date  2026-06-24\n"
                                 "Amount  CNY 1299.00\n"
                                 "Please review the recognized text on the right.");
    result.regions = {
        OcrTextRegion{QRect(126, 122, 412, 42), QStringLiteral("Invoice #A-240624"), 96},
        OcrTextRegion{QRect(126, 196, 506, 34), QStringLiteral("Customer  Sunny Capturer Studio"),
                      94},
        OcrTextRegion{QRect(126, 250, 256, 34), QStringLiteral("Date  2026-06-24"), 95},
        OcrTextRegion{QRect(126, 304, 304, 34), QStringLiteral("Amount  CNY 1299.00"), 93},
        OcrTextRegion{QRect(126, 410, 698, 34),
                      QStringLiteral("Please review the recognized text on the right."), 91},
    };
    return result;
}

} // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    const QString outputPath =
        argc >= 2 ? QString::fromLocal8Bit(argv[1]) : QStringLiteral("/tmp/cappy-ocr-result.png");
    const QString appearance =
        argc >= 3 ? QString::fromLocal8Bit(argv[2]).trimmed().toLower() : QStringLiteral("dark");
    const QString language =
        argc >= 4 ? QString::fromLocal8Bit(argv[3]).trimmed() : QStringLiteral("zh-CN");

    cappy::services::ocr::OcrSettings settings;
    settings.preferredProvider = "local";
    settings.localCommand = "tesseract";
    settings.localLanguage = "eng+chi_sim";

    cappy::features::ocr::OcrResultWindow window(
        buildPreviewImage(), settings,
        language.compare(QStringLiteral("en"), Qt::CaseInsensitive) == 0
            ? cappy::localization::AppLanguage::English
            : cappy::localization::AppLanguage::SimplifiedChinese,
        appearance, nullptr, buildMockResult(), false);
    window.show();
    app.processEvents();

    if (auto* resultEdit = window.findChild<QPlainTextEdit*>()) {
        QTextCursor cursor(resultEdit->document()->findBlockByNumber(2));
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        resultEdit->setTextCursor(cursor);
        resultEdit->centerCursor();
        app.processEvents();
    }

    window.resize(1180, 760);
    app.processEvents();

    QImage image(window.size() * window.devicePixelRatioF(), QImage::Format_ARGB32_Premultiplied);
    image.setDevicePixelRatio(window.devicePixelRatioF());
    image.fill(Qt::transparent);

    QPainter painter(&image);
    window.render(&painter);
    painter.end();

    QFileInfo outputInfo(outputPath);
    QDir().mkpath(outputInfo.absolutePath());
    image.save(outputPath);
    return 0;
}
