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

private:
    void paintEvent(QPaintEvent *);

    QImage m_image;
    Mode m_mode;
    QSvgRenderer *m_svgRenderer;
};

#endif // PREVIEWWIDGET_H
