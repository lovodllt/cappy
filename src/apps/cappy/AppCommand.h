#pragma once

enum class AppCommand {
    ShowHome,
    StartRegionCapture,
    StartCurrentScreenCapture,
    StartFullscreenCapture,
    StartActiveWindowCapture,
    StartWindowFitCapture,
    PinLatestCapture,
    SaveLatestCapture,
    CloseAllPins,
    EnablePinClickThrough,
    RestorePinInput,
    OpenCapturesDirectory,
    OpenSettings,
    Restart,
    HideToTray,
    Quit,
};
