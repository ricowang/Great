#include "greatwindow.h"
#include "ui_greatwindow.h"
#include "viewmanager.h"
#include <QCloseEvent>
#include "contentmanager.h"
#include "downloadmanager.h"
#include "canvas.h"
#include <QDesktopWidget>

GreatWindow::GreatWindow(int monitor, bool main, QWidget *parent) :
    QMainWindow(parent), m_monitor(monitor),
    ui(new Ui::GreatWindow), m_bMainWindow(main)
{
    ui->setupUi(this);
    if(!main) {
        ui->menuBar->hide();
        ui->statusBar->hide();
    }
    ui->mainToolBar->hide();

    m_canvas = new CCanvas(ui->center);
    this->setCentralWidget(m_canvas);
    m_jumping = false;

    connect(&m_timer, &QTimer::timeout, this, &GreatWindow::checkShift);
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

}

void GreatWindow::setImageActSize(const QSize &sz)
{
    m_canvas->setImageActSize(sz);
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
}

void GreatWindow::reverFullScreen()
{
    if(Qt::WindowFullScreen != windowState()) {
         m_geoData = saveGeometry();
         if(m_bMainWindow) {
             ui->menuBar->hide();
             ui->statusBar->hide();
         }
         showFullScreen();
    } else {
        if(m_bMainWindow) {
            ui->menuBar->show();
            ui->statusBar->show();
        }
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

    m_mainUrl = ContentMgr.getUrl(-numSteps, true, false);
    if(!m_jumping) {
        ViewMgr.present(m_mainUrl);
    }
}

void GreatWindow::checkShift()
{

}

void GreatWindow::loaderProgress(CDecoder *loader, DecoderStatus status)
{
    if(DS_Finished == status && m_decoder.get() == loader) {
        display(loader->image());
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

