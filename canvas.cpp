#include "canvas.h"
#include <QPainter>

CCanvas::CCanvas(QWidget *parent) : QWidget(parent)
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

void CCanvas::draw(QPainter* painter)
{
    if(m_image.width() == 0 || m_image.height() == 0)
    {
        return;
    }

    QRect picRect(0, 0, m_pic_size.width(), m_pic_size.height());
    QRect wndRect = painter->window();

    painter->setClipRect(wndRect, Qt::IntersectClip);
    painter->drawImage(m_offset, m_image, picRect);
}

void CCanvas::setImage(const QImage &img)
{
    m_image = img;
    m_offset = QPoint(0, 0);
    offset(0, 0, true);
}

void CCanvas::setImageActSize(const QSize& sz)
{
    m_pic_size = sz;
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

