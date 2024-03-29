#ifndef DXCBBACKINGSTORE_H
#define DXCBBACKINGSTORE_H

#define private public
#include <qpa/qplatformbackingstore.h>
#undef private

#include <QBasicTimer>

QT_BEGIN_NAMESPACE
class QXcbBackingStore;
class QWidgetWindow;
QT_END_NAMESPACE

class DXcbShmGraphicsBuffer;
class WindowEventListener;

struct xcb_property_notify_event_t;

class DXcbBackingStore : public QPlatformBackingStore
{
public:
    DXcbBackingStore(QWindow *window, QXcbBackingStore *proxy);
    ~DXcbBackingStore();

    QPaintDevice *paintDevice() Q_DECL_OVERRIDE;

    // 'window' can be a child window, in which case 'region' is in child window coordinates and
    // offset is the (child) window's offset in relation to the window surface.
    void flush(QWindow *window, const QRegion &region, const QPoint &offset) Q_DECL_OVERRIDE;
#ifndef QT_NO_OPENGL
    void composeAndFlush(QWindow *window, const QRegion &region, const QPoint &offset,
                                 QPlatformTextureList *textures, QOpenGLContext *context,
                                 bool translucentBackground) Q_DECL_OVERRIDE;
    QImage toImage() const Q_DECL_OVERRIDE;

    GLuint toTexture(const QRegion &dirtyRegion, QSize *textureSize, TextureFlags *flags) const Q_DECL_OVERRIDE;
#endif

    QPlatformGraphicsBuffer *graphicsBuffer() const Q_DECL_OVERRIDE;

    void resize(const QSize &size, const QRegion &staticContents) Q_DECL_OVERRIDE;

    void beginPaint(const QRegion &region) Q_DECL_OVERRIDE;
    void endPaint() Q_DECL_OVERRIDE;

    static bool isWidgetWindow(const QWindow *window);

private:
    void initUserPropertys();

    void updateWindowMargins();
    void updateFrameExtents();
    void updateInputShapeRegion();
    void updateClipPath();
    void updateWindowShadow();
    void doDelayedUpdateWindowShadow(int delaye = 100);

    /// update of user propertys
    void updateWindowRadius();
    void updateBorderWidth();
    void updateBorderColor();
    void updateUserClipPath();
    void updateFrameMask();
    void updateShadowRadius();
    void updateShadowOffset();
    void updateShadowColor();

    void setWindowMargins(const QMargins &margins);
    void setClipPah(const QPainterPath &path);

    void paintWindowShadow(QRegion region = QRegion());
    void repaintWindowShadow();

    inline bool isWidgetWindow() const
    { return isWidgetWindow(window());}
    QWidgetWindow *widgetWindow() const;

    inline QPoint windowOffset() const
    { return QPoint(windowMargins.left(), windowMargins.top());}
    inline QRect windowGeometry() const
    { return QRect(windowOffset(), m_image.size());}

    bool canUseClipPath() const;

    void onWindowStateChanged();
    void handlePropertyNotifyEvent(const xcb_property_notify_event_t *event);

    QSize m_size;
    QImage m_image;

    QXcbBackingStore *m_proxy;
    WindowEventListener *m_eventListener;
    DXcbShmGraphicsBuffer *m_graphicsBuffer = Q_NULLPTR;

    int m_windowRadius = 10;
    int m_borderWidth = 1;
    bool isUserSetClipPath = false;
    QPainterPath m_clipPath;
    QPainterPath m_windowClipPath;
    QColor m_borderColor = QColor(255, 0, 0, 255 * 0.5);

    int m_shadowRadius = 20;//40;
    QPoint m_shadowOffset = QPoint(0, 0);
    QColor m_shadowColor = Qt::black;//QColor(0, 0, 0, 255 * 0.5);
    QPixmap shadowPixmap;

    QRect windowValidRect;
    QMargins windowMargins;

    bool isUserSetFrameMask = false;

    QBasicTimer updateShadowTimer;

    /// Settings window margins before the window position deviation
    QPoint oldWindowOffset;

    friend class WindowEventListener;
};

#endif // DXCBBACKINGSTORE_H
