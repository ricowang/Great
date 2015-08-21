#include "downloadmanager.h"
#include "loader.h"
#include "viewmanager.h"
#include "QApplication"
CDownloadManager* CDownloadManager::s_poInstance = nullptr;

CDownloadManager* CDownloadManager::getInstance()
{
    if(nullptr == s_poInstance) {
       s_poInstance = new CDownloadManager();
    }

    return s_poInstance;
}

std::shared_ptr<CDecoder> CDownloadManager::start(const QString &url)
{
    // check if it's loaded or not
    for(auto it=m_decoders.begin(); it!=m_decoders.end(); it++) {
        auto thisUrl = (*it)->url();
        if( thisUrl == url) {
            // redownload
            if((*it)->status() ==  DS_Aborted) {
                m_decoders.erase(it);
                break;
            } else {
                return *it;
            }
        }
    }

    // test file type as local
    std::shared_ptr<CLoader> loader;

    if(url.mid(0, 4).toLower() == "http") {
        loader = std::make_shared<CNetworkLoader>(url);
    } else {
        loader = std::make_shared<CFileLoader>(url);
    }

    m_decoders.push_back(std::make_shared<CDecoder>(loader));
    return (*m_decoders.rbegin());
}


void CDownloadManager::loaderProgress(CDecoder* loader, DecoderStatus status)
{
    if(DS_HasHeader == status) {
        auto size = loader->fullSize();
        auto wind = ViewMgr.findWindow(size);
        wind->raise();
        wind->activateWindow();
        QApplication::setActiveWindow(wind);

        for(auto it=m_decoders.begin(); it!=m_decoders.end(); it++) {
            if( (*it).get() == loader) {
                wind->setLoader(*it);
                wind->setImageActSize(loader->fullSize());
                break;
            }
        }
    }

    if(DS_Finished == status) {
        for(auto it=m_decoders.begin(); it!=m_decoders.end(); it++) {
            if( (*it).get() == loader) {
                m_decoders.erase(it);
                break;
            }
        }
    }
}

CDownloadManager::CDownloadManager()
{

}

CDownloadManager::~CDownloadManager()
{

}
