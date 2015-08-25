#include "greatwindow.h"
#include "ui_greatwindow.h"
#include "viewmanager.h"
#include <QCloseEvent>
#include "contentmanager.h"
#include "downloadmanager.h"
#include "canvas.h"
#include <QDesktopWidget>
#include <contentmanager.h>
#include <QLabel>
#include <QPushButton>

GreatWindow::GreatWindow(int monitor, bool main, QWidget *parent) :
    QMainWindow(parent), m_monitor(monitor),
    ui(new Ui::GreatWindow), m_bMainWindow(main)
{
    ui->setupUi(this);
    if(!main) {
        ui->menuBar->hide();
    } else {
        m_recentFileActions[0] = ui->actionFile1;
        m_recentFileActions[1] = ui->actionFile2;
        m_recentFileActions[2] = ui->actionFile3;
        m_recentFileActions[3] = ui->actionFile4;
        m_recentFileActions[4] = ui->actionFile5;

        for(int i=0; i<5; i++){
            connect(m_recentFileActions[i], SIGNAL(triggered()),
                    this, SLOT(openRecentFile()));
        }

        ContentMgr.loadRecentFiles();
    }

    QLabel* statusLabel = new QLabel("");
    statusLabel->setAlignment(Qt::AlignLeft);
    statusLabel->setMinimumWidth(100);

    QLabel* resolutionLabel = new QLabel("");
    resolutionLabel->setAlignment(Qt::AlignLeft);
    resolutionLabel->setMinimumWidth(150);

    QLabel* zoomLabel = new QLabel("");
    zoomLabel->setAlignment(Qt::AlignLeft);
    zoomLabel->setMinimumWidth(100);

    QLabel* urlLabel = new QLabel("");
    urlLabel->setAlignment(Qt::AlignLeft);
    urlLabel->setMinimumWidth(400);

    QPushButton* zoomInButton = new QPushButton(QString(0x2295));
    zoomInButton->setMaximumWidth(urlLabel->sizeHint().height());
    zoomInButton->setMaximumHeight(urlLabel->sizeHint().height());

    QPushButton* zoomOutButton = new QPushButton(QString(0x2296));
    zoomOutButton->setMaximumWidth(urlLabel->sizeHint().height());
    zoomOutButton->setMaximumHeight(urlLabel->sizeHint().height());

    statusBar()->addWidget(statusLabel);
    statusBar()->addWidget(resolutionLabel);
    statusBar()->addWidget(zoomLabel);
    statusBar()->addWidget(zoomInButton);
    statusBar()->addWidget(zoomOutButton);
    statusBar()->addWidget(urlLabel, 1);

    m_statusLabels[0] = statusLabel;
    m_statusLabels[1] = resolutionLabel;
    m_statusLabels[2] = zoomLabel;
    m_statusLabels[3] = urlLabel;

    ui->mainToolBar->hide();
    updateRecent();

    m_canvas = new CCanvas(ui->center);
    this->setCentralWidget(m_canvas);
    m_jumping = false;

    connect(zoomInButton, &QPushButton::clicked,
            m_canvas, [this] {m_canvas->zoom(true); updateZoom(); });
    connect(zoomOutButton, &QPushButton::clicked,
            m_canvas, [this] {m_canvas->zoom(false); updateZoom(); });

    updateZoom();
    //connect(&m_timer, &QTimer::timeout, this, &GreatWindow::checkShift);
    //m_timer.start(250);
}

GreatWindow::~GreatWindow()
{
    delete ui;
}

void GreatWindow::on_action_Quit_triggered()
{
    ViewMgr.quit();
}

void GreatWindow::closeEvent(QCloseEvent *event)
{
    ViewMgr.quitOne(this);
    event->accept();
}

void GreatWindow::display(const QImage &img)
{
    m_canvas->setImage(img);
    m_lasts.clear();
}

void GreatWindow::setLoader(std::shared_ptr<CDecoder> decoder)
{
    m_lasts.push_back(m_decoder);
    m_decoder = decoder;
    connect(decoder.get(), &CDecoder::statusChange,
            this, &GreatWindow::loaderProgress);
    setStatus(3, m_decoder->url());
    setStatus(1, "");
    setStatus(0, m_decoder->statusText());
}

void GreatWindow::setImageActSize(const QSize &sz)
{
    m_canvas->setImageActSize(sz);
}

void GreatWindow::updateRecent()
{
    if(!m_bMainWindow) {
        return;
    }

    for(int i=0; i<5; i++) {
        QString path = ContentMgr.getRecent(i);
        if(path.isEmpty()) {
            m_recentFileActions[i]->setVisible(false);
        } else {
            m_recentFileActions[i]->setData(path);

            int idx = path.lastIndexOf("/")+1;
            m_recentFileActions[i]->setText(path.mid(idx));
            m_recentFileActions[i]->setVisible(true);
        }
    }
}

void GreatWindow::setStatus(int id, const QString &msg)
{
    if(id<0 || id>3) {
        return;
    }

    m_statusLabels[id]->setText(msg);
}

float GreatWindow::updateZoom()
{
     double zoom = m_canvas->getZoom();
     QString msg = QString("%1%").arg(QString::number((int)(zoom*100)));
     setStatus(2, msg);
}

#define PressedAny(k) (QString(k).indexOf(char(key)) != -1)
void GreatWindow::keyPressEvent(QKeyEvent *event)
{
    // nNpPmMoO
    int key=event->key();
    if(PressedAny("nNpPmMoO ")) {
        if(event->modifiers() & Qt::ShiftModifier)
        {
            m_jumping = true;
        }

        int offset = 1;
        bool group = false;
        if(QString("oOpP").indexOf(char(key)) != -1) {
            offset = -1;
        }

        if(QString("mMpP").indexOf(char(key)) != -1) {
            group = true;
        }

        m_mainUrl = ContentMgr.getUrl(offset, true, group);
        if(!m_jumping) {
            ViewMgr.present(m_mainUrl);
        }

        return;
    }

    if(PressedAny("Aa")) {
        bool fit = m_canvas->getFit();
        m_canvas->setFit(!fit);
        updateZoom();
    }

    if(PressedAny("Qq")) {
        on_action_Quit_triggered();
    }

#define DELTA 20
    if(PressedAny("jklhJKLH")) {
        int x = 0, y = 0;
        if(key == 'h' || key == 'H') {
            x = DELTA;
        }

        if(key == 'l' || key == 'L') {
            x = -DELTA;
        }

        if(key == 'j' || key == 'J') {
            y = -DELTA;
        }

        if(key == 'k' || key == 'K') {
            y = DELTA;
        }

        m_canvas->offset(x, y);
    }

    if(PressedAny("fF")) {
        reverFullScreen();
    }

    if(PressedAny("sS")) {
        ViewMgr.activeNext(this);
    }

    if(PressedAny("dD")) {
        ViewMgr.setDuplicate();
    }

    if(PressedAny("=+-0")) {
        float zoom = 0.0;
        if('0' == key) {
            zoom = m_canvas->setZoom(1);
        } else {
            zoom = m_canvas->zoom(key!='-'?true:false);
        }

        updateZoom();
    }
}

void GreatWindow::reverFullScreen()
{
    if(Qt::WindowFullScreen != windowState()) {
         m_geoData = saveGeometry();
         if(m_bMainWindow) {
             ui->menuBar->hide();
         }
         ui->statusBar->hide();
         showFullScreen();
    } else {
        if(m_bMainWindow) {
            ui->menuBar->show();
        }
        ui->statusBar->show();
        showNormal();
        restoreGeometry(m_geoData);
    }
}

void GreatWindow::keyReleaseEvent(QKeyEvent *event)
{
    int key=event->key();
    if(m_jumping && PressedAny("nNpPmMoO") &&
            !(event->modifiers() & Qt::ShiftModifier)) {
        m_jumping = false;
        ViewMgr.present(m_mainUrl);
    }
}

void GreatWindow::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;
    m_jumping = event->modifiers() & Qt::ShiftModifier;
    auto zooming = event->modifiers() & Qt::ControlModifier;

    if(zooming) {
        while(numSteps!= 0) {
            m_canvas->zoom(numSteps>0);
            numSteps += numSteps>0?-1:1;
            updateZoom();
        }
        return;
    }

    m_mainUrl = ContentMgr.getUrl(-numSteps, true, false);
    if(!m_jumping) {
        ViewMgr.present(m_mainUrl);
    }
}

void GreatWindow::mousePressEvent(QMouseEvent *event)
{
    m_lastPos = event->pos();
    m_mousePressed = true;
    this->setCursor(Qt::OpenHandCursor);
}

void GreatWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_mousePressed = false;
    this->setCursor(Qt::ArrowCursor);
}

void GreatWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_mousePressed) {
        return;
    }

    QPoint newPoint = event->pos();
    int deltaX = newPoint.x() - m_lastPos.x();
    int deltaY = newPoint.y() - m_lastPos.y();

    if((deltaX*deltaX + deltaY*deltaY) > 625) {
        m_lastPos = newPoint;
        m_canvas->offset(deltaX, deltaY);
    }
}

void GreatWindow::checkShift()
{

}

void GreatWindow::loaderProgress(CDecoder *loader, DecoderStatus status)
{
    if(m_decoder.get() == loader) {
        setStatus(0, m_decoder->statusText());
    }

    if(DS_Finished == status && m_decoder.get() == loader) {
        QSize size = m_decoder->size();
        setStatus(1, QString("%1x%2").arg(QString::number(size.width()),
                                          QString::number(size.height())));
        display(loader->image());
    }
}

void GreatWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        QString file = action->data().toString();

        if(!ContentMgr.openFile(file))
        {
            return;
        }

        QString next = ContentMgr.getUrl(0);
        ViewMgr.present(next);
    }
}


void GreatWindow::on_action_Open_triggered()
{
    if(!ContentMgr.open(this)) {
        return;
    }

    QString next = ContentMgr.getUrl(0);
    ViewMgr.present(next);
}

