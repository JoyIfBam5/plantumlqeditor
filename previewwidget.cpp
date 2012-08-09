#include "previewwidget.h"
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_mode(NoMode)
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
    update();
}

void PreviewWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if (m_mode == PngMode) {
        painter.drawImage(QPoint(0, 0), m_image);
    } else if (m_mode == SvgMode) {
        QRect output_rect(QPoint(), m_svgRenderer->defaultSize());
        QSize adjusted_size = output_rect.size();
        adjusted_size.scale(rect().size(), Qt::KeepAspectRatio);
        m_svgRenderer->render(&painter, QRectF(QPointF(), adjusted_size));
    }
}
