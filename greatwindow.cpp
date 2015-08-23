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

    QLabel* urlLabel = new QLabel("");
    urlLabel->setAlignment(Qt::AlignRight);
    urlLabel->setMinimumWidth(400);

    QLabel* statusLabel = new QLabel("");
    statusLabel->setAlignment(Qt::AlignLeft);
    statusLabel->setMinimumWidth(100);

    QLabel* resolutionLabel = new QLabel("");
    resolutionLabel->setAlignment(Qt::AlignLeft);
    resolutionLabel->setMinimumWidth(150);

    statusBar()->addWidget(statusLabel);
    statusBar()->addWidget(resolutionLabel);
    statusBar()->addWidget(urlLabel, 1);
    m_statusLabels[0] = statusLabel;
    m_statusLabels[1] = resolutionLabel;
    m_statusLabels[2] = urlLabel;

    ui->mainToolBar->hide();
    updateRecent();

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
    setStatus(2, m_decoder->url());
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
    if(id<0 || id>2) {
        return;
    }

    m_statusLabels[id]->setText(msg);
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

    if(PressedAny("+-")) {
        m_canvas->zoom(key=='+'?true:false);
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

