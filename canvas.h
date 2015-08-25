#ifndef CCANVAS_H
#define CCANVAS_H

#include <QWidget>

class CCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit CCanvas(QWidget *parent = 0);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent * event);

    void draw(QPainter* painter);

    void setImage(const QImage& img);
    void setImageActSize(const QSize& sz);
    void offset(int x, int y, bool repaint = false);
    double zoom(bool in);
    double setZoom(float f);
    double getZoom() { return m_zoom/100.0; }
    bool setFit(bool fit);
    bool getFit() { return m_zoomFit; }

protected:
    QImage    m_image;
    QSize     m_pic_size;
    QSize     m_org_size;
    QPoint    m_offset;
    int       m_zoom;
    bool      m_zoomFit;

signals:

public slots:
};

#endif // CCANVAS_H
