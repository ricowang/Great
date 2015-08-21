#ifndef CCANVAS_H
#define CCANVAS_H

#include <QWidget>

class CCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit CCanvas(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    void draw(QPainter* painter);

    void setImage(const QImage& img);
    void setImageActSize(const QSize& sz);
    void offset(int x, int y, bool repaint = false);

protected:
    QImage    m_image;
    QSize     m_pic_size;
    QPoint    m_offset;

signals:

public slots:
};

#endif // CCANVAS_H
