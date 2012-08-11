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
        QSize output_size = m_image.size();
        QRect output_rect(QPoint(), output_size);
        output_rect.translate(rect().center() - output_rect.center());
        painter.drawImage(output_rect.topLeft(), m_image);
    } else if (m_mode == SvgMode) {
        QSize output_size = m_svgRenderer->defaultSize();
        output_size.scale(rect().size(), Qt::KeepAspectRatio);
        QRect output_rect(QPoint(), output_size);
        output_rect.translate(rect().center() - output_rect.center());
        m_svgRenderer->render(&painter, output_rect);
    }
}
