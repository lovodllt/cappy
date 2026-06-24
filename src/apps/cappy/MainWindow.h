#pragma once

#include <QAction>
#include <QFrame>
#include <QImage>
#include <QLabel>
#include <QListWidget>
#include <QPoint>
#include <QStringList>
#include <QMainWindow>

#include "cappy/localization/Localization.h"
#include "cappy/shortcuts/ShortcutSettings.h"

class MainWindow final : public QMainWindow {
    Q_OBJECT

  public:
    struct CaptureHistoryEntry {
        QString id;
        QString title;
        QImage image;
        QString filePath;
        int captureMode = -1;
    };

    explicit MainWindow(const QStringList& pluginIds, QWidget* parent = nullptr);

    void applyAppearanceMode(const QString& appearanceMode);
    void applyLanguage(cappy::localization::AppLanguage language);
    void applyShortcutSettings(const cappy::shortcuts::MainWindowShortcutSettings& shortcuts);
    void setShortcutActionsSuspended(bool suspended);
    void setCloseToTrayEnabled(bool enabled);
    void setCommandStatus(const QString& status);
    void setHistoryLimit(int limit);
    void showCaptureResult(const QImage& image, const QRect& geometry, const QString& backendName);
    void addCaptureHistoryEntry(const CaptureHistoryEntry& entry);
    void updateHistoryEntryImage(const QString& entryId, const QImage& image,
                                 bool clearSavedState = false);
    void markHistoryEntrySaved(const QString& entryId, const QString& filePath);

  signals:
    void regionCaptureRequested();
    void fullscreenCaptureRequested();
    void activeWindowCaptureRequested();
    void windowFitCaptureRequested();
    void pinLatestCaptureRequested();
    void saveLatestCaptureRequested();
    void closeAllPinsRequested();
    void restorePinInputRequested();
    void historyPinRequested(const QImage& image);
    void historySaveRequested(const QString& entryId, const QImage& image, int captureMode);
    void historyCopyRequested(const QImage& image);
    void openCapturesDirectoryRequested();
    void settingsRequested();
    void hideToTrayRequested();
    void quitRequested();

  protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

  private:
    QAction* createWindowAction(const QIcon& icon, const QString& text,
                                const QKeySequence& shortcut = {});
    void syncPreviewToCurrentContext();
    void updatePreviewPixmap();
    void updateHistorySummary();
    [[nodiscard]] QListWidgetItem* selectedHistoryItem() const;
    void pinSelectedHistoryItem();
    void copySelectedHistoryItem();
    void saveSelectedHistoryItem();
    void removeSelectedHistoryItem();
    void updateHistoryActionState();
    void showHistoryContextMenu(const QPoint& position);
    void updateHistoryItemDisplay(QListWidgetItem* item);
    void trimHistoryToLimit();
    [[nodiscard]] QList<QAction*> shortcutActions() const;
    void refreshActionTexts();

    bool closeToTrayEnabled_ = true;
    int historyLimit_ = 10;
    cappy::localization::AppLanguage language_ = cappy::localization::AppLanguage::English;
    cappy::shortcuts::MainWindowShortcutSettings mainWindowShortcuts_;
    QImage latestCaptureImage_;
    QImage previewImage_;
    QLabel* titleLabel_ = nullptr;
    QLabel* workspaceTitleLabel_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* previewLabel_ = nullptr;
    QLabel* historyTitleLabel_ = nullptr;
    QLabel* historySummaryLabel_ = nullptr;
    QListWidget* historyList_ = nullptr;
    QAction* regionCaptureAction_ = nullptr;
    QAction* fullscreenCaptureAction_ = nullptr;
    QAction* activeWindowCaptureAction_ = nullptr;
    QAction* windowFitCaptureAction_ = nullptr;
    QAction* pinLatestAction_ = nullptr;
    QAction* saveLatestAction_ = nullptr;
    QAction* closePinsAction_ = nullptr;
    QAction* restorePinInputAction_ = nullptr;
    QAction* openFolderAction_ = nullptr;
    QAction* settingsAction_ = nullptr;
    QAction* trayAction_ = nullptr;
    QAction* quitAction_ = nullptr;
    QAction* pinSelectedHistoryAction_ = nullptr;
    QAction* copySelectedHistoryAction_ = nullptr;
    QAction* saveSelectedHistoryAction_ = nullptr;
    QAction* removeSelectedHistoryAction_ = nullptr;
};
