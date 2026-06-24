#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QPainter>

#include "MainWindow.h"

namespace {

QImage makeImage(const QColor& color, const QSize& size) {
    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(color);

    QPainter painter(&image);
    painter.setPen(QPen(QColor(255, 255, 255, 90), 2));
    painter.drawRoundedRect(image.rect().adjusted(2, 2, -3, -3), 8, 8);
    painter.end();
    return image;
}

}  // namespace

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    const QString outputPath = argc >= 2
        ? QString::fromLocal8Bit(argv[1])
        : QStringLiteral("/tmp/cappy-main-window.png");
    const QString appearance = argc >= 3
        ? QString::fromLocal8Bit(argv[2]).trimmed().toLower()
        : QStringLiteral("dark");
    const QString language = argc >= 4
        ? QString::fromLocal8Bit(argv[3]).trimmed()
        : QStringLiteral("zh-CN");

    MainWindow window({}, nullptr);
    window.applyAppearanceMode(appearance);
    window.applyLanguage(
        language.compare(QStringLiteral("en"), Qt::CaseInsensitive) == 0
            ? cappy::localization::AppLanguage::English
            : cappy::localization::AppLanguage::SimplifiedChinese
    );

    window.addCaptureHistoryEntry(
        {.id = "entry-1",
         .title = "Region 1",
         .image = makeImage(QColor(82, 134, 209), QSize(320, 180)),
         .filePath = {},
         .captureMode = 0}
    );
    window.addCaptureHistoryEntry(
        {.id = "entry-2",
         .title = "Screen 1",
         .image = makeImage(QColor(83, 163, 122), QSize(320, 180)),
         .filePath = "/tmp/cappy-demo.png",
         .captureMode = 3}
    );

    window.resize(1180, 760);
    window.show();
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
