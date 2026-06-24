#include "QtScreenCaptureBackend.h"

#include <cstring>
#include <cstdlib>

#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QCursor>
#include <QtMath>

#include "cappy/domain/capture/CaptureImageOps.h"

#if defined(Q_OS_LINUX)
#include <QtGui/qguiapplication_platform.h>
#include <xcb/xcb.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace cappy::platform::capture {

namespace {

#if defined(Q_OS_LINUX)
xcb_atom_t internAtom(xcb_connection_t* connection, const char* name) {
    const xcb_intern_atom_cookie_t cookie =
        xcb_intern_atom(connection, 0, static_cast<uint16_t>(std::strlen(name)), name);
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(connection, cookie, nullptr);
    if (reply == nullptr) {
        return XCB_ATOM_NONE;
    }

    const xcb_atom_t atom = reply->atom;
    free(reply);
    return atom;
}

bool windowHasProperty(xcb_connection_t* connection, xcb_window_t window, xcb_atom_t propertyAtom) {
    if (connection == nullptr || window == XCB_WINDOW_NONE || propertyAtom == XCB_ATOM_NONE) {
        return false;
    }

    const xcb_get_property_cookie_t propertyCookie =
        xcb_get_property(connection, 0, window, propertyAtom, XCB_GET_PROPERTY_TYPE_ANY, 0, 0);
    xcb_get_property_reply_t* propertyReply =
        xcb_get_property_reply(connection, propertyCookie, nullptr);
    if (propertyReply == nullptr) {
        return false;
    }

    const bool hasProperty = propertyReply->type != XCB_ATOM_NONE;
    free(propertyReply);
    return hasProperty;
}

QRect geometryForWindow(xcb_connection_t* connection, xcb_window_t root, xcb_window_t window) {
    if (connection == nullptr || window == XCB_WINDOW_NONE) {
        return {};
    }

    const xcb_get_geometry_cookie_t geometryCookie = xcb_get_geometry(connection, window);
    xcb_get_geometry_reply_t* geometryReply =
        xcb_get_geometry_reply(connection, geometryCookie, nullptr);

    const xcb_translate_coordinates_cookie_t translateCookie =
        xcb_translate_coordinates(connection, window, root, 0, 0);
    xcb_translate_coordinates_reply_t* translateReply =
        xcb_translate_coordinates_reply(connection, translateCookie, nullptr);

    QRect geometry;
    if (geometryReply != nullptr && translateReply != nullptr) {
        geometry = QRect(translateReply->dst_x, translateReply->dst_y, geometryReply->width,
                         geometryReply->height);
    }

    free(geometryReply);
    free(translateReply);
    return geometry;
}

xcb_window_t findManagedClientWindow(xcb_connection_t* connection, xcb_window_t window,
                                     xcb_atom_t wmStateAtom, int depth = 3) {
    if (connection == nullptr || window == XCB_WINDOW_NONE || depth < 0) {
        return XCB_WINDOW_NONE;
    }

    if (windowHasProperty(connection, window, wmStateAtom)) {
        return window;
    }

    const xcb_query_tree_cookie_t treeCookie = xcb_query_tree(connection, window);
    xcb_query_tree_reply_t* treeReply = xcb_query_tree_reply(connection, treeCookie, nullptr);
    if (treeReply == nullptr) {
        return XCB_WINDOW_NONE;
    }

    const int childCount = xcb_query_tree_children_length(treeReply);
    xcb_window_t* children = xcb_query_tree_children(treeReply);
    xcb_window_t managedWindow = XCB_WINDOW_NONE;
    for (int index = childCount - 1; index >= 0; --index) {
        managedWindow =
            findManagedClientWindow(connection, children[index], wmStateAtom, depth - 1);
        if (managedWindow != XCB_WINDOW_NONE) {
            break;
        }
    }

    free(treeReply);
    return managedWindow;
}

xcb_window_t topLevelWindowAtPoint(xcb_connection_t* connection, xcb_window_t root,
                                   const QPoint& point, xcb_window_t excludedWindow) {
    if (connection == nullptr || root == XCB_WINDOW_NONE) {
        return XCB_WINDOW_NONE;
    }

    const xcb_query_tree_cookie_t treeCookie = xcb_query_tree(connection, root);
    xcb_query_tree_reply_t* treeReply = xcb_query_tree_reply(connection, treeCookie, nullptr);
    if (treeReply == nullptr) {
        return XCB_WINDOW_NONE;
    }

    xcb_window_t selectedWindow = XCB_WINDOW_NONE;
    const int childCount = xcb_query_tree_children_length(treeReply);
    xcb_window_t* children = xcb_query_tree_children(treeReply);

    for (int index = childCount - 1; index >= 0; --index) {
        const xcb_window_t candidate = children[index];
        if (candidate == XCB_WINDOW_NONE || candidate == excludedWindow) {
            continue;
        }

        const xcb_get_window_attributes_cookie_t attributesCookie =
            xcb_get_window_attributes(connection, candidate);
        xcb_get_window_attributes_reply_t* attributesReply =
            xcb_get_window_attributes_reply(connection, attributesCookie, nullptr);
        if (attributesReply == nullptr) {
            continue;
        }

        const bool isViewable = attributesReply->map_state == XCB_MAP_STATE_VIEWABLE;
        free(attributesReply);
        if (!isViewable) {
            continue;
        }

        const QRect geometry = geometryForWindow(connection, root, candidate);
        if (geometry.contains(point)) {
            selectedWindow = candidate;
            break;
        }
    }

    free(treeReply);
    return selectedWindow;
}

QRect captureGeometryForWindow(xcb_connection_t* connection, xcb_window_t root,
                               xcb_window_t window) {
    if (connection == nullptr || root == XCB_WINDOW_NONE || window == XCB_WINDOW_NONE) {
        return {};
    }

    const xcb_atom_t wmStateAtom = internAtom(connection, "WM_STATE");
    const xcb_window_t managedWindow = findManagedClientWindow(connection, window, wmStateAtom);
    if (managedWindow != XCB_WINDOW_NONE) {
        const QRect managedGeometry = geometryForWindow(connection, root, managedWindow);
        if (!managedGeometry.isEmpty()) {
            return managedGeometry;
        }
    }

    return geometryForWindow(connection, root, window);
}
#endif

} // namespace

QString QtScreenCaptureBackend::backendName() const {
    return "qt-screen";
}

bool QtScreenCaptureBackend::isSupported() const {
    return !QGuiApplication::screens().isEmpty();
}

QString QtScreenCaptureBackend::unsupportedReason() const {
    if (isSupported()) {
        return {};
    }

    return "No screens available";
}

cappy::domain::capture::DesktopFrame QtScreenCaptureBackend::captureVirtualDesktop() const {
    cappy::domain::capture::DesktopFrame frame;
    const QList<QScreen*> screens = QGuiApplication::screens();
    if (screens.isEmpty()) {
        return frame;
    }

    QRect virtualGeometry;
    for (QScreen* screen : screens) {
        virtualGeometry = virtualGeometry.united(screen->geometry());
    }

    if (virtualGeometry.isEmpty()) {
        return frame;
    }

    QImage composed(virtualGeometry.size(), QImage::Format_ARGB32_Premultiplied);
    composed.fill(Qt::transparent);

    QPainter painter(&composed);
    for (QScreen* screen : screens) {
        const QRect screenGeometry = screen->geometry();
        const QPoint targetTopLeft = screenGeometry.topLeft() - virtualGeometry.topLeft();
        const QPixmap pixmap = screen->grabWindow(0);
        const QImage screenImage = pixmap.toImage();
        frame.screenFragments.push_back(cappy::domain::capture::ScreenFragment{
            .image = screenImage,
            .geometry = screenGeometry,
        });
        painter.drawImage(QRect(targetTopLeft, screenGeometry.size()), screenImage);
    }
    painter.end();

    frame.image = composed;
    frame.geometry = virtualGeometry;
    return frame;
}

QRect QtScreenCaptureBackend::activeWindowGeometry() const {
    return queryActiveWindowGeometry();
}

QRect QtScreenCaptureBackend::windowGeometryAtPoint(const QPoint& point,
                                                    WId excludedWindowId) const {
#if defined(Q_OS_LINUX)
    if (QGuiApplication::platformName() != "xcb") {
        return {};
    }

    auto* nativeApp = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (nativeApp == nullptr) {
        return {};
    }

    xcb_connection_t* connection = nativeApp->connection();
    if (connection == nullptr) {
        return {};
    }

    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_iterator_t screenIterator = xcb_setup_roots_iterator(setup);
    if (screenIterator.rem == 0 || screenIterator.data == nullptr) {
        return {};
    }

    xcb_screen_t* screen = screenIterator.data;
    const xcb_window_t window = topLevelWindowAtPoint(connection, screen->root, point,
                                                      static_cast<xcb_window_t>(excludedWindowId));
    if (window == XCB_WINDOW_NONE) {
        return {};
    }

    return captureGeometryForWindow(connection, screen->root, window);
#elif defined(Q_OS_WIN)
    POINT nativePoint{point.x(), point.y()};
    const HWND excludedWindow = reinterpret_cast<HWND>(excludedWindowId);
    HWND window = WindowFromPoint(nativePoint);
    while (window != nullptr) {
        if (window != excludedWindow && IsWindowVisible(window)) {
            RECT rect{};
            if (GetWindowRect(window, &rect) != 0) {
                return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
            }
        }
        window = GetAncestor(window, GA_ROOTOWNER);
        if (window == excludedWindow) {
            break;
        }
        if (window != nullptr && !IsWindowVisible(window)) {
            break;
        }
        if (window != nullptr && excludedWindow != nullptr && window == excludedWindow) {
            break;
        }
    }
    return {};
#else
    return {};
#endif
}

cappy::domain::capture::CaptureResult QtScreenCaptureBackend::captureCurrentScreen() const {
    cappy::domain::capture::CaptureResult result;
    result.mode = cappy::domain::capture::CaptureMode::CurrentScreen;
    result.backendName = backendName();

    QScreen* screen = QGuiApplication::screenAt(QCursor::pos());
    if (screen == nullptr) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen == nullptr) {
        return result;
    }

    const QPixmap pixmap = screen->grabWindow(0);
    const QImage image = pixmap.toImage();
    if (image.isNull()) {
        return result;
    }

    result.geometry = screen->geometry();
    result.image = image;
    return result;
}

cappy::domain::capture::CaptureResult QtScreenCaptureBackend::captureActiveWindow() const {
    cappy::domain::capture::CaptureResult result;
    result.mode = cappy::domain::capture::CaptureMode::ActiveWindow;
    result.backendName = backendName();

    const QRect activeWindowGeometry = this->activeWindowGeometry();
    if (activeWindowGeometry.isEmpty()) {
        return result;
    }

    const cappy::domain::capture::DesktopFrame frame = captureVirtualDesktop();
    if (frame.isNull()) {
        return result;
    }

    const QImage croppedImage =
        cappy::domain::capture::cropNormalizedImage(frame, activeWindowGeometry);
    if (croppedImage.isNull()) {
        return result;
    }

    result.geometry = activeWindowGeometry;
    result.image = croppedImage;
    return result;
}

QRect QtScreenCaptureBackend::queryActiveWindowGeometry() const {
#if defined(Q_OS_LINUX)
    if (QGuiApplication::platformName() != "xcb") {
        return {};
    }

    auto* nativeApp = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (nativeApp == nullptr) {
        return {};
    }

    xcb_connection_t* connection = nativeApp->connection();
    if (connection == nullptr) {
        return {};
    }

    const xcb_setup_t* setup = xcb_get_setup(connection);
    xcb_screen_iterator_t screenIterator = xcb_setup_roots_iterator(setup);
    if (screenIterator.rem == 0 || screenIterator.data == nullptr) {
        return {};
    }

    xcb_screen_t* screen = screenIterator.data;
    const xcb_atom_t activeWindowAtom = internAtom(connection, "_NET_ACTIVE_WINDOW");
    if (activeWindowAtom == XCB_ATOM_NONE) {
        return {};
    }

    const xcb_get_property_cookie_t propertyCookie =
        xcb_get_property(connection, 0, screen->root, activeWindowAtom, XCB_ATOM_WINDOW, 0, 1);
    xcb_get_property_reply_t* propertyReply =
        xcb_get_property_reply(connection, propertyCookie, nullptr);
    if (propertyReply == nullptr) {
        return {};
    }

    QRect geometry;
    if (xcb_get_property_value_length(propertyReply) >= static_cast<int>(sizeof(xcb_window_t))) {
        const auto* windowData = static_cast<xcb_window_t*>(xcb_get_property_value(propertyReply));
        const xcb_window_t activeWindow = *windowData;

        if (activeWindow != XCB_WINDOW_NONE) {
            geometry = captureGeometryForWindow(connection, screen->root, activeWindow);
        }
    }

    free(propertyReply);
    return geometry;
#elif defined(Q_OS_WIN)
    const HWND activeWindow = GetForegroundWindow();
    if (activeWindow == nullptr || !IsWindowVisible(activeWindow)) {
        return {};
    }

    RECT rect{};
    if (GetWindowRect(activeWindow, &rect) == 0) {
        return {};
    }

    return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
#else
    return {};
#endif
}

} // namespace cappy::platform::capture
