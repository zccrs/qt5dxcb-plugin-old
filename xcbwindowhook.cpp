#include "xcbwindowhook.h"
#include "vtablehook.h"
#include "global.h"
#include "utility.h"

#include <private/qwindow_p.h>

#define HOOK_VFPTR(Fun) VtableHook::overrideVfptrFun(window, &QPlatformWindow::Fun, this, &XcbWindowHook::Fun)
#define CALL this->window()->QXcbWindow

QHash<const QPlatformWindow*, XcbWindowHook*> XcbWindowHook::mapped;

XcbWindowHook::XcbWindowHook(QXcbWindow *window)
    : xcbWindow(window)
{
    mapped[window] = this;

    HOOK_VFPTR(setGeometry);
    HOOK_VFPTR(geometry);
//    HOOK_VFPTR(frameMargins);
//    HOOK_VFPTR(setParent);
//    HOOK_VFPTR(setWindowTitle);
//    HOOK_VFPTR(setWindowIcon);
//    HOOK_VFPTR(mapToGlobal);
//    HOOK_VFPTR(mapFromGlobal);
    HOOK_VFPTR(setMask);
    HOOK_VFPTR(propagateSizeHints);

    QObject::connect(window->window(), &QWindow::destroyed, window->window(), [this] {
        if (mapped.contains(xcbWindow))
            delete this;
    });
}

XcbWindowHook::~XcbWindowHook()
{
    mapped.remove(xcbWindow);
}

XcbWindowHook *XcbWindowHook::me() const
{
    return getHookByWindow(window());
}

void XcbWindowHook::setGeometry(const QRect &rect)
{
    const QMargins &margins = me()->windowMargins;

//    qDebug() << __FUNCTION__ << rect << rect + margins;

    CALL::setGeometry(rect + margins);
}

QRect XcbWindowHook::geometry() const
{
    const QMargins &margins = me()->windowMargins;

//    qDebug() << __FUNCTION__ << CALL::geometry() << CALL::window()->isVisible();

    return CALL::geometry() - margins;
}

QMargins XcbWindowHook::frameMargins() const
{
    QMargins margins = CALL::frameMargins();

    return margins/* + me()->windowMargins*/;
}

void XcbWindowHook::setParent(const QPlatformWindow *window)
{
    CALL::setParent(window);
}

void XcbWindowHook::setWindowTitle(const QString &title)
{
    return CALL::setWindowTitle(title);
}

void XcbWindowHook::setWindowIcon(const QIcon &icon)
{
    return CALL::setWindowIcon(icon);
}

//QPoint XcbWindowHook::mapToGlobal(const QPoint &pos) const
//{
//    XcbWindowHook *me = XcbWindowHook::me();

//    return CALL::mapToGlobal(pos + QPoint(me->windowMargins.left(), me->windowMargins.top()));
//}

//QPoint XcbWindowHook::mapFromGlobal(const QPoint &pos) const
//{
//    XcbWindowHook *me = XcbWindowHook::me();

//    return CALL::mapFromGlobal(pos - QPoint(me->windowMargins.left(), me->windowMargins.top()));
//}

void XcbWindowHook::setMask(const QRegion &region)
{
    QRegion tmp_region;

    const QMargins &margins = me()->windowMargins;

    QRect window_rect = CALL::geometry() - margins;

    window_rect.moveTopLeft(QPoint(margins.left(), margins.top()));

    for (const QRect &rect : region.rects()) {
        tmp_region += rect.translated(window_rect.topLeft()).intersected(window_rect) + margins;
    }

    QPainterPath path;

    path.addRegion(region);

    CALL::window()->setProperty(clipPath, QVariant::fromValue(path));
//    CALL::setMask(tmp_region);
    Utility::setInputShapeRectangles(CALL::winId(), tmp_region);
}

void XcbWindowHook::propagateSizeHints()
{
    QWindow *win = window()->window();
    QWindowPrivate *winp = qt_window_private(win);

    const QMargins &windowMarings = me()->windowMargins;

    const QSize &marginSize = QSize(windowMarings.left() + windowMarings.right(),
                                    windowMarings.top() + windowMarings.bottom());

    winp->minimumSize += marginSize;
    winp->maximumSize += marginSize;
    winp->maximumSize.setWidth(qMin(QWINDOWSIZE_MAX, winp->maximumSize.width()));
    winp->maximumSize.setHeight(qMin(QWINDOWSIZE_MAX, winp->maximumSize.height()));

    CALL::propagateSizeHints();
}

XcbWindowHook *XcbWindowHook::getHookByWindow(const QPlatformWindow *window)
{
    return mapped.value(window);
}
