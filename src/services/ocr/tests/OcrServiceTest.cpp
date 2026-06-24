#include <QtTest>

#include <QFile>
#include <QImage>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTextStream>

#include "cappy/services/ocr/OcrService.h"

class OcrServiceTest final : public QObject {
    Q_OBJECT

private slots:
    void localRecognitionParsesStructuredTsvOutput();
};

void OcrServiceTest::localRecognitionParsesStructuredTsvOutput() {
#ifdef Q_OS_WIN
    QSKIP("This test currently uses a POSIX shell fixture.");
#endif

    qRegisterMetaType<cappy::services::ocr::OcrResult>();

    QTemporaryDir temporaryDir;
    QVERIFY(temporaryDir.isValid());

    const QString scriptPath = temporaryDir.filePath(QStringLiteral("fake-tesseract.sh"));
    QFile scriptFile(scriptPath);
    QVERIFY(scriptFile.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream scriptStream(&scriptFile);
    scriptStream << "#!/usr/bin/env bash\n";
    scriptStream << "printf '%s\\n' \\\n";
    scriptStream << "'level\tpage_num\tblock_num\tpar_num\tline_num\tword_num\tleft\ttop\twidth\theight\tconf\ttext' \\\n";
    scriptStream << "'5\t1\t1\t1\t1\t1\t10\t20\t40\t18\t95\tHello' \\\n";
    scriptStream << "'5\t1\t1\t1\t1\t2\t58\t20\t26\t18\t93\tOCR' \\\n";
    scriptStream << "'5\t1\t1\t1\t2\t1\t12\t52\t20\t20\t88\t你' \\\n";
    scriptStream << "'5\t1\t1\t1\t2\t2\t36\t52\t20\t20\t87\t好'\n";
    scriptFile.close();
    QVERIFY(scriptFile.setPermissions(
        QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner
    ));

    cappy::services::ocr::OcrService service;
    QSignalSpy finishedSpy(&service, &cappy::services::ocr::OcrService::finished);
    QSignalSpy failedSpy(&service, &cappy::services::ocr::OcrService::failed);

    cappy::services::ocr::OcrSettings settings;
    settings.localCommand = scriptPath;
    settings.localLanguage.clear();

    QImage image(120, 80, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::white);

    service.recognize(image, settings, cappy::services::ocr::OcrProvider::Local);

    QVERIFY(finishedSpy.wait(3000));
    QCOMPARE(failedSpy.count(), 0);
    QCOMPARE(finishedSpy.count(), 1);

    const auto result =
        qvariant_cast<cappy::services::ocr::OcrResult>(finishedSpy.at(0).at(0));
    QCOMPARE(result.text, QString("Hello OCR\n你好"));
    QCOMPARE(result.regions.size(), 2);
    QCOMPARE(result.regions.at(0).text, QString("Hello OCR"));
    QCOMPARE(result.regions.at(0).rect, QRect(10, 20, 74, 18));
    QCOMPARE(result.regions.at(1).text, QString("你好"));
    QCOMPARE(result.regions.at(1).rect, QRect(12, 52, 44, 20));
}

QTEST_MAIN(OcrServiceTest)

#include "OcrServiceTest.moc"
