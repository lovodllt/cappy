#include <QtTest>

#include <QToolButton>

#include "MainWindow.h"

namespace {

QImage makeImage(const QColor& color) {
    QImage image(32, 24, QImage::Format_ARGB32_Premultiplied);
    image.fill(color);
    return image;
}

}  // namespace

class MainWindowTest final : public QObject {
    Q_OBJECT

private slots:
    void historyLimitTrimsOldEntries();
    void workspaceDoesNotRenderCaptureCommandButtons();
};

void MainWindowTest::historyLimitTrimsOldEntries() {
    MainWindow window({}, nullptr);
    window.setHistoryLimit(2);

    window.addCaptureHistoryEntry(
        {.id = "entry-1",
         .title = "First",
         .image = makeImage(Qt::red),
         .filePath = {},
         .captureMode = 0}
    );
    window.addCaptureHistoryEntry(
        {.id = "entry-2",
         .title = "Second",
         .image = makeImage(Qt::green),
         .filePath = {},
         .captureMode = 1}
    );
    window.addCaptureHistoryEntry(
        {.id = "entry-3",
         .title = "Third",
         .image = makeImage(Qt::blue),
         .filePath = {},
         .captureMode = 2}
    );

    auto* historyList = window.findChild<QListWidget*>();
    QVERIFY(historyList != nullptr);
    QCOMPARE(historyList->count(), 2);
    QCOMPARE(historyList->item(0)->data(Qt::UserRole + 3).toString(), QString("entry-3"));
    QCOMPARE(historyList->item(1)->data(Qt::UserRole + 3).toString(), QString("entry-2"));
}

void MainWindowTest::workspaceDoesNotRenderCaptureCommandButtons() {
    MainWindow window({}, nullptr);
    window.show();
    QCoreApplication::processEvents();

    const auto buttons = window.findChildren<QToolButton*>();
    bool foundTextButton = false;
    for (QToolButton* button : buttons) {
        if (button == nullptr) {
            continue;
        }
        if (button->toolButtonStyle() == Qt::ToolButtonTextBesideIcon) {
            foundTextButton = true;
            break;
        }
    }

    QVERIFY(!foundTextButton);
}

QTEST_MAIN(MainWindowTest)

#include "MainWindowTest.moc"
