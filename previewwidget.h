#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QImage>

class QSvgRenderer;

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    enum Mode { NoMode, PngMode, SvgMode };

    explicit PreviewWidget(QWidget *parent = 0);

    Mode mode() const { return m_mode; }
    void setMode(Mode new_mode) { m_mode = new_mode; }

    void load(const QByteArray &data);

public slots:
    void zoomOriginal() { setZoomScale(ZOOM_ORIGINAL_SCALE); }
    void zoomIn();
    void zoomOut();

private:
    static const int ZOOM_ORIGINAL_SCALE = 100;

    void paintEvent(QPaintEvent *);
    void zoomImage();
    void setZoomScale(int new_scale);

    QImage m_image;
    QImage m_zoomedImage;
    Mode m_mode;
    QSvgRenderer *m_svgRenderer;
    int m_zoomScale;
};

#endif // PREVIEWWIDGET_H
