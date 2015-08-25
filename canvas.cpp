#include "canvas.h"
#include <QPainter>

CCanvas::CCanvas(QWidget *parent) :
    QWidget(parent), m_zoom(100), m_zoomFit(false)
{
    QPalette Pal(palette());
    Pal.setColor(QPalette::Background, Qt::black);
    setAutoFillBackground(true);
    setPalette(Pal);
}

void CCanvas::paintEvent(QPaintEvent * /*event*/ )
{
    QPainter painter(this);
    draw(&painter);
}

void CCanvas::resizeEvent(QResizeEvent *event)
{
    if(m_zoomFit) {
        setFit(true);
    } else {
        offset(0, 0, true);
    }
}

void CCanvas::draw(QPainter* painter)
{
    if(m_image.width() == 0 || m_image.height() == 0)
    {
        return;
    }

    QRect destRect(0, 0, m_pic_size.width(), m_pic_size.height());
    QRect wndRect = painter->window();
    QRect srcRect(0, 0, m_org_size.width(), m_org_size.height());

    painter->setClipRect(wndRect, Qt::IntersectClip);
    destRect.moveTo(m_offset.x(), m_offset.y());
    painter->drawImage(destRect, m_image, srcRect);
    //painter->drawImage(m_offset, m_image, destRect);

}

void CCanvas::setImage(const QImage &img)
{
    m_image = img;
    m_offset = QPoint(0, 0);
    offset(0, 0, true);
}

void CCanvas::setImageActSize(const QSize& sz)
{
    if(m_zoomFit) {
        double r1 = double(size().width())/sz.width();
        double r2 = double(size().height())/sz.height();

        int zoom = 100*(r1<r2?r1:r2);
        if(m_zoom != zoom) {
            m_zoom = zoom;
        }
    }

    m_org_size = sz;
    m_pic_size.setWidth(m_org_size.width()*m_zoom/100.0);
    m_pic_size.setHeight(m_org_size.height()*m_zoom/100.0);
}

void CCanvas::offset(int x, int y, bool rp)
{
    if(m_pic_size.isEmpty()) {
        return;
    }

    QSize wnd_size = this->parentWidget()->size();
    auto old_offset = m_offset;

    if(m_pic_size.width() < wnd_size.width()) {
        m_offset.rx() = (wnd_size.width()-m_pic_size.width())/2;
    } else {
        m_offset.rx() += x;

        if(m_offset.x() > 0) {
            m_offset.rx() = 0;
        }

        if((m_offset.x() + m_pic_size.width()) < wnd_size.width()) {
            m_offset.rx() = wnd_size.width() - m_pic_size.width();
        }
    }

    if(m_pic_size.height() < wnd_size.height()) {
        m_offset.ry() = (wnd_size.height()-m_pic_size.height())/2;
    } else {
        m_offset.ry() += y;

        if(m_offset.y() > 0) {
            m_offset.ry() = 0;
        }

        if((m_offset.y() + m_pic_size.height()) < wnd_size.height()) {
            m_offset.ry() = wnd_size.height() - m_pic_size.height();
        }
    }

    if(old_offset != m_offset || rp) {
        repaint();
    }
}

double CCanvas::setZoom(float f)
{
    if(m_zoomFit) {
        return m_zoom/100.0;
    }

    m_zoom = f>=0?f*100:100;

    if(m_org_size.width()*m_org_size.height() > 0)
    {
        m_pic_size.setWidth(m_org_size.width()*m_zoom/100.0);
        m_pic_size.setHeight(m_org_size.height()*m_zoom/100.0);
        offset(0, 0, true);
    }

    return m_zoom/100.0;
}

bool CCanvas::setFit(bool fit)
{
    m_zoomFit = fit;
    if(!fit) {
        m_zoom = 100;
    }
    setImageActSize(m_org_size);
    m_offset = QPoint(0, 0);
    offset(0, 0, true);
}

double CCanvas::zoom(bool in)
{
    if(m_zoomFit) {
        return m_zoom/100.0;
    }

    m_zoom += in?10:-10;
    m_zoom = m_zoom<=0?10:m_zoom;

    if(m_org_size.width()*m_org_size.height() > 0)
    {
        m_pic_size.setWidth(m_org_size.width()*m_zoom/100.0);
        m_pic_size.setHeight(m_org_size.height()*m_zoom/100.0);
        offset(0, 0, true);
    }

    return m_zoom/100.0;
}

