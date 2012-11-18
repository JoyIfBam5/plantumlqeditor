#include "previewwidget.h"
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>

namespace {
    const int ZOOM_BIG_INCREMENT = 200;  // used when m_zoomScale > ZOOM_ORIGINAL_SCALE
    const int ZOOM_SMALL_INCREMENT = 25; // used when m_zoomScale < ZOOM_ORIGINAL_SCALE
    const int MAX_ZOOM_SCALE = 900;
    const int MIN_ZOOM_SCALE = 25;
}
PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_mode(NoMode)
    , m_zoomScale(ZOOM_ORIGINAL_SCALE)
{
    m_svgRenderer = new QSvgRenderer(this);
}

void PreviewWidget::load(const QByteArray &data)
{
    if (m_mode == PngMode) {
        m_image.loadFromData(data);
        setMinimumSize(m_image.rect().size());
    } else if (m_mode == SvgMode) {
        m_svgRenderer->load(data);
    }
    zoomImage();
    update();
}

void PreviewWidget::setZoomScale(int zoom_scale)
{
    if (m_zoomScale != zoom_scale) {
        m_zoomScale = zoom_scale;
        zoomImage();
        update();
    }
}

void PreviewWidget::zoomIn()
{
    int new_scale = m_zoomScale + ((m_zoomScale >= ZOOM_ORIGINAL_SCALE) ? ZOOM_BIG_INCREMENT : ZOOM_SMALL_INCREMENT);
    if (new_scale > MAX_ZOOM_SCALE)
        new_scale = MAX_ZOOM_SCALE;
    setZoomScale(new_scale);
}

void PreviewWidget::zoomOut()
{
    int new_scale = m_zoomScale - ((m_zoomScale <= ZOOM_ORIGINAL_SCALE) ? ZOOM_SMALL_INCREMENT : ZOOM_BIG_INCREMENT);
    if (new_scale < MIN_ZOOM_SCALE)
        new_scale = MIN_ZOOM_SCALE;
    setZoomScale(new_scale);
}

void PreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QSize output_size;
    if (m_mode == PngMode) {
        output_size = m_zoomedImage.size();
        QRect output_rect(QPoint(), output_size);
        output_rect.translate(rect().center() - output_rect.center());
        painter.drawImage(output_rect.topLeft(), m_zoomedImage);
    } else if (m_mode == SvgMode) {
        output_size = m_svgRenderer->defaultSize();
        if (m_zoomScale != ZOOM_ORIGINAL_SCALE) {
            float zoom = float(m_zoomScale) / ZOOM_ORIGINAL_SCALE;
            output_size.scale(output_size.width() * zoom, output_size.height() * zoom, Qt::IgnoreAspectRatio);
        }
        QRect output_rect(QPoint(), output_size);
        output_rect.translate(rect().center() - output_rect.center());
        m_svgRenderer->render(&painter, output_rect);
    }
    setMinimumSize(output_size);
}

void PreviewWidget::zoomImage()
{
    if (m_mode == PngMode) {
        if (m_zoomScale == ZOOM_ORIGINAL_SCALE) {
            m_zoomedImage = m_image;
        } else {
            float zoom = float(m_zoomScale) / ZOOM_ORIGINAL_SCALE;
            m_zoomedImage = m_image.scaled(m_image.width() * zoom, m_image.height() * zoom,
                                           Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }
    }
}
