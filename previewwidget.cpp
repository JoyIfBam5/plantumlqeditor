#include "previewwidget.h"
#include <QPainter>
#include <QDebug>

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
{
}

void PreviewWidget::loadImage(const QByteArray &data)
{
    m_image.loadFromData(data);
    qDebug() << "new image rect:" << m_image.rect();
    setMinimumSize(m_image.rect().size());
    resize(m_image.rect().size());
    adjustSize();
    update();
}

void PreviewWidget::paintEvent(QPaintEvent *)
{
    if (!m_image.isNull()) {
         QPainter painter(this);
         painter.setBrush(Qt::Dense6Pattern);
         painter.drawRect(m_image.rect().adjusted(0, 0, -1, -1));
         painter.drawImage(QPoint(0, 0), m_image);
     }
}
