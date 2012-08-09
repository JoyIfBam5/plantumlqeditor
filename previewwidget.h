#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>
#include <QImage>

class PreviewWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = 0);

    void loadImage(const QByteArray& data);

private:
    void paintEvent(QPaintEvent *);

    QImage m_image;
};

#endif // PREVIEWWIDGET_H
