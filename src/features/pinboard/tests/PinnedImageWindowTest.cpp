#include <QCoreApplication>
#include <QImage>
#include <QPoint>
#include <QtTest>

#include "cappy/features/pinboard/PinnedImageWindow.h"

namespace {

constexpr int kShadowMargin = 6;

QImage makeTestImage() {
    QImage image(80, 50, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(120, 180, 240));
    return image;
}

QImage makeReplacementImage() {
    QImage image(96, 64, QImage::Format_ARGB32_Premultiplied);
    image.fill(QColor(210, 120, 140));
    return image;
}

}  // namespace

class PinnedImageWindowTest : public QObject {
    Q_OBJECT

private slots:
    void honorsRequestedInitialTopLeft();
    void restoreResetsImageState();
    void restoreReturnsToInitialImageAfterReplacingImage();
};

void PinnedImageWindowTest::honorsRequestedInitialTopLeft() {
    const QPoint expectedTopLeft(120, 84);
    cappy::features::pinboard::PinnedImageWindow window(makeTestImage(), expectedTopLeft);

    window.show();
    QCoreApplication::processEvents();

    QCOMPARE(window.pos(), expectedTopLeft - QPoint(kShadowMargin, kShadowMargin));
    QCOMPARE(window.size(), QSize(80 + kShadowMargin * 2, 50 + kShadowMargin * 2));
    QCOMPARE(window.imageRect().topLeft(), QPoint(kShadowMargin, kShadowMargin));
    QCOMPARE(window.imageRect().size(), QSize(80, 50));
}

void PinnedImageWindowTest::restoreResetsImageState() {
    cappy::features::pinboard::PinnedImageWindow window(makeTestImage(), QPoint(40, 30));

    window.rotateClockwise();
    window.invertColors();
    window.setScaleFactor(1.5);
    window.setWindowOpacityLevel(0.4);
    window.setLocked(true);

    QCOMPARE(window.size(), QSize(75 + kShadowMargin * 2, 120 + kShadowMargin * 2));
    QVERIFY(window.locked_);
    QVERIFY(window.image_ != window.originalImage_);

    window.restoreOriginalState();

    QCOMPARE(window.size(), QSize(80 + kShadowMargin * 2, 50 + kShadowMargin * 2));
    QCOMPARE(window.image_, window.originalImage_);
    QCOMPARE(window.scaleFactor_, 1.0);
    QCOMPARE(window.opacityLevel_, 1.0);
    QVERIFY(!window.locked_);
}

void PinnedImageWindowTest::restoreReturnsToInitialImageAfterReplacingImage() {
    cappy::features::pinboard::PinnedImageWindow window(makeTestImage(), QPoint(40, 30));

    const QImage replacement = makeReplacementImage();
    window.replaceImageForEditing(replacement);
    QCOMPARE(window.image_, replacement);
    QCOMPARE(window.originalImage_, replacement);
    QCOMPARE(window.initialImage_, makeTestImage());

    window.rotateClockwise();
    QVERIFY(window.image_ != window.initialImage_);

    window.restoreOriginalState();

    QCOMPARE(window.image_, window.initialImage_);
    QCOMPARE(window.originalImage_, window.initialImage_);
    QCOMPARE(window.size(), QSize(80 + kShadowMargin * 2, 50 + kShadowMargin * 2));
}

QTEST_MAIN(PinnedImageWindowTest)

#include "PinnedImageWindowTest.moc"
