#include <QApplication>
#include <QCoreApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>
#include <QPoint>
#include <QtTest>

#include "cappy/domain/capture/CaptureTypes.h"
#include "cappy/features/capture/CaptureOverlayWidget.h"

namespace {

using cappy::domain::capture::CaptureMode;
using cappy::domain::capture::CaptureResult;
using cappy::domain::capture::DesktopFrame;
using cappy::features::capture::CaptureFinalizeAction;
using cappy::features::capture::CaptureOverlayWidget;

void sendMouseEvent(CaptureOverlayWidget& widget, QEvent::Type type, const QPoint& position,
                    Qt::MouseButton button, Qt::MouseButtons buttons) {
    QMouseEvent event(type, QPointF(position), QPointF(position), QPointF(position), button,
                      buttons, Qt::NoModifier);
    QApplication::sendEvent(&widget, &event);
}

void dragSelection(CaptureOverlayWidget& widget, const QPoint& start, const QPoint& end) {
    sendMouseEvent(widget, QEvent::MouseButtonPress, start, Qt::LeftButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseMove, end, Qt::NoButton, Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, end, Qt::LeftButton, Qt::NoButton);
}

DesktopFrame makeDesktopFrame() {
    QImage image(320, 200, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(32, 96, 160));

    DesktopFrame frame;
    frame.image = image;
    frame.geometry = QRect(0, 0, image.width(), image.height());
    return frame;
}

} // namespace

class CaptureOverlayWidgetTest : public QObject {
    Q_OBJECT

  private slots:
    void dragSelectionAndFinalizeByEnterEmitsCapture();
    void dragSelectionAndFinalizeByPlainSaveShortcutEmitsSave();
    void initialSelectionFinalizePreservesCaptureMode();
    void trackedWindowClickConfirmsWindowFitSelection();
    void trackedWindowDragFallsBackToRegionSelection();
};

void CaptureOverlayWidgetTest::dragSelectionAndFinalizeByEnterEmitsCapture() {
    CaptureOverlayWidget widget(makeDesktopFrame());

    bool finalized = false;
    CaptureResult finalizedResult;
    CaptureFinalizeAction finalizedAction = CaptureFinalizeAction::Save;
    QObject::connect(&widget, &CaptureOverlayWidget::captureFinalized, this,
                     [&](const CaptureResult& result, CaptureFinalizeAction action) {
                         finalized = true;
                         finalizedResult = result;
                         finalizedAction = action;
                     });

    widget.show();
    widget.raise();
    widget.activateWindow();
    widget.setFocus(Qt::ActiveWindowFocusReason);
    QCoreApplication::processEvents();

    dragSelection(widget, QPoint(20, 24), QPoint(180, 120));

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(&widget, &keyPress);
    QCoreApplication::processEvents();

    QVERIFY(finalized);
    QCOMPARE(finalizedAction, CaptureFinalizeAction::Copy);
    QCOMPARE(finalizedResult.mode, CaptureMode::Region);
    QCOMPARE(finalizedResult.geometry, QRect(20, 24, 161, 97));
    QVERIFY(!finalizedResult.image.isNull());
    QCOMPARE(finalizedResult.image.size(), QSize(161, 97));
}

void CaptureOverlayWidgetTest::dragSelectionAndFinalizeByPlainSaveShortcutEmitsSave() {
    CaptureOverlayWidget widget(makeDesktopFrame());

    bool finalized = false;
    CaptureFinalizeAction finalizedAction = CaptureFinalizeAction::Copy;
    QObject::connect(&widget, &CaptureOverlayWidget::captureFinalized, this,
                     [&](const CaptureResult&, CaptureFinalizeAction action) {
                         finalized = true;
                         finalizedAction = action;
                     });

    widget.show();
    widget.raise();
    widget.activateWindow();
    widget.setFocus(Qt::ActiveWindowFocusReason);
    QCoreApplication::processEvents();

    dragSelection(widget, QPoint(30, 40), QPoint(200, 140));

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_S, Qt::NoModifier);
    QApplication::sendEvent(&widget, &keyPress);
    QCoreApplication::processEvents();

    QVERIFY(finalized);
    QCOMPARE(finalizedAction, CaptureFinalizeAction::Save);
}

void CaptureOverlayWidgetTest::initialSelectionFinalizePreservesCaptureMode() {
    CaptureOverlayWidget widget(makeDesktopFrame(), {}, cappy::localization::AppLanguage::English,
                                QRect(40, 50, 120, 80), CaptureMode::WindowFit);

    bool finalized = false;
    CaptureResult finalizedResult;
    QObject::connect(&widget, &CaptureOverlayWidget::captureFinalized, this,
                     [&](const CaptureResult& result, CaptureFinalizeAction) {
                         finalized = true;
                         finalizedResult = result;
                     });

    widget.show();
    widget.raise();
    widget.activateWindow();
    widget.setFocus(Qt::ActiveWindowFocusReason);
    QCoreApplication::processEvents();

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(&widget, &keyPress);
    QCoreApplication::processEvents();

    QVERIFY(finalized);
    QCOMPARE(finalizedResult.mode, CaptureMode::WindowFit);
    QCOMPARE(finalizedResult.geometry, QRect(40, 50, 120, 80));
    QCOMPARE(finalizedResult.image.size(), QSize(120, 80));
}

void CaptureOverlayWidgetTest::trackedWindowClickConfirmsWindowFitSelection() {
    CaptureOverlayWidget widget(makeDesktopFrame(), {}, cappy::localization::AppLanguage::English,
                                std::nullopt, CaptureMode::WindowFit,
                                [](const QPoint&, WId) { return QRect(60, 70, 140, 90); });

    bool finalized = false;
    CaptureResult finalizedResult;
    QObject::connect(&widget, &CaptureOverlayWidget::captureFinalized, this,
                     [&](const CaptureResult& result, CaptureFinalizeAction) {
                         finalized = true;
                         finalizedResult = result;
                     });

    widget.show();
    widget.raise();
    widget.activateWindow();
    widget.setFocus(Qt::ActiveWindowFocusReason);
    QCoreApplication::processEvents();

    sendMouseEvent(widget, QEvent::MouseMove, QPoint(80, 90), Qt::NoButton, Qt::NoButton);
    sendMouseEvent(widget, QEvent::MouseButtonPress, QPoint(80, 90), Qt::LeftButton,
                   Qt::LeftButton);
    sendMouseEvent(widget, QEvent::MouseButtonRelease, QPoint(80, 90), Qt::LeftButton,
                   Qt::NoButton);

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(&widget, &keyPress);
    QCoreApplication::processEvents();

    QVERIFY(finalized);
    QCOMPARE(finalizedResult.mode, CaptureMode::WindowFit);
    QCOMPARE(finalizedResult.geometry, QRect(60, 70, 140, 90));
}

void CaptureOverlayWidgetTest::trackedWindowDragFallsBackToRegionSelection() {
    CaptureOverlayWidget widget(makeDesktopFrame(), {}, cappy::localization::AppLanguage::English,
                                std::nullopt, CaptureMode::WindowFit,
                                [](const QPoint&, WId) { return QRect(60, 70, 140, 90); });

    bool finalized = false;
    CaptureResult finalizedResult;
    QObject::connect(&widget, &CaptureOverlayWidget::captureFinalized, this,
                     [&](const CaptureResult& result, CaptureFinalizeAction) {
                         finalized = true;
                         finalizedResult = result;
                     });

    widget.show();
    widget.raise();
    widget.activateWindow();
    widget.setFocus(Qt::ActiveWindowFocusReason);
    QCoreApplication::processEvents();

    sendMouseEvent(widget, QEvent::MouseMove, QPoint(80, 90), Qt::NoButton, Qt::NoButton);
    dragSelection(widget, QPoint(80, 90), QPoint(190, 160));

    QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(&widget, &keyPress);
    QCoreApplication::processEvents();

    QVERIFY(finalized);
    QCOMPARE(finalizedResult.mode, CaptureMode::Region);
    QCOMPARE(finalizedResult.geometry, QRect(80, 90, 111, 71));
}

QTEST_MAIN(CaptureOverlayWidgetTest)

#include "CaptureOverlayWidgetTest.moc"
