#include "viewmanager.h"

#include <QDesktopWidget>
#include <QApplication>
#include "greatwindow.h"
#include "downloadmanager.h"

CViewManager* CViewManager::s_poInstance = nullptr;

CViewManager* CViewManager::getInstance()
{
    if(nullptr == s_poInstance) {
        s_poInstance = new CViewManager();
    }

    return s_poInstance;
}

CViewManager::CViewManager(QObject* parent):
    QObject(parent), m_bSarted(false), m_bDuplicated(false)
{

}

CViewManager::~CViewManager()
{

}

void CViewManager::start()
{
    QDesktopWidget* pDesk = QApplication::desktop();
    const int primary = pDesk->primaryScreen();
    std::shared_ptr<GreatWindow> window;

    for(int i=0; i<pDesk->screenCount(); i++) {
        QRect screenres = pDesk->screenGeometry(i);

         window = std::make_shared<GreatWindow>(i, primary == i);
         screenres = pDesk->screenGeometry(i);
         window->move(QPoint(screenres.x(), screenres.y()));
         window->resize(screenres.width(), screenres.height());

         if(primary != i) {
            m_vWindows.push_back(window);
         } else {
            m_vWindows.insert(m_vWindows.begin(), window);
         }
    }

    m_bSarted = true;
}

void CViewManager::show()
{
    for(auto ix = m_vWindows.rbegin(); ix != m_vWindows.rend(); ix++) {
        (*ix)->show();
    }
}

void CViewManager::quit()
{
    QCoreApplication::quit();
}

void CViewManager::quitOne(const GreatWindow* pWnd)
{
    for(auto ix = m_vWindows.begin(); ix != m_vWindows.end(); ix++) {
        if((*ix).get() == pWnd) {
            m_vWindows.erase((ix));
            break;
        }
    }
}

void CViewManager::activeNext(const GreatWindow *pWnd)
{
    bool next = false;
    for(auto ix = m_vWindows.begin(); ix != m_vWindows.end(); ix++) {
        if((*ix).get() == pWnd) {
            next = true;
            continue;
        }

        if(next) {
            (*ix)->raise();
            (*ix)->activateWindow();
            QApplication::setActiveWindow((*ix).get());
            return;
        }
    }

    (*m_vWindows.begin())->activateWindow();
    (*m_vWindows.begin())->raise();

}

void CViewManager::updateRecent()
{
    for(auto ix = m_vWindows.begin(); ix != m_vWindows.end(); ix++) {
        (*ix)->updateRecent();
    }
}

void CViewManager::present(const QString& url)
{
    if(url.isEmpty()) {
        return;
    }

    auto decoder = DownloadMgr.start(url);

    if(decoder->status() == DS_NotStarted) {

        connect(decoder.get(), &CDecoder::statusChange,
                &DownloadMgr, &CDownloadManager::loaderProgress,
                Qt::QueuedConnection);

        decoder->start();
    } else if(decoder->status() == DS_Finished) {
        DownloadMgr.loaderProgress(decoder.get(), DS_HasHeader);
        DownloadMgr.loaderProgress(decoder.get(), DS_Finished);
    }
}

GreatWindow *CViewManager::findWindow(QSize psz)
{
    // if any window fit all
    for(auto it=m_vWindows.begin(); it<m_vWindows.end(); it++) {
        QSize wsz = (*it)->size();
        float zoom = (*it)->updateZoom();
        wsz.setWidth(wsz.width()/zoom);
        wsz.setHeight(wsz.height()/zoom);

        if(psz.width() <= wsz.width() && psz.height() <= wsz.height())  {
            return (*it).get();
        }
    }

    GreatWindow* rt = (*m_vWindows.begin()).get();
    QSize rrs = QSize(0, 0);

    for(auto it=m_vWindows.begin(); it<m_vWindows.end(); it++) {
        auto wsz = (*it)->size();
        float zoom = (*it)->updateZoom();
        wsz.setWidth(wsz.width()*zoom);
        wsz.setHeight(wsz.height()*zoom);

        auto irs = QRect(QPoint(0, 0), wsz).intersected(
                    QRect(QPoint(0, 0), psz)).size();

        if(rrs.width()*rrs.height() < irs.width()*irs.height()) {
            rt = (*it).get();
            rrs = irs;
        }
    }

    return rt;
}

void CViewManager::setDuplicate()
{

}
