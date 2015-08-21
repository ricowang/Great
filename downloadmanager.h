#ifndef CDOWNLOADMANAGER_H
#define CDOWNLOADMANAGER_H
#include <QString>
#include <QNetworkAccessManager>
#include <vector>
#include <memory>
#include "decoder.h"
#include <QObject>

class CDownloadManager : public QObject
{
    Q_OBJECT

 public:
    explicit CDownloadManager();
    virtual ~CDownloadManager();
    static CDownloadManager* getInstance();

    std::shared_ptr<CDecoder> start(const QString& url);

public slots:
    void loaderProgress(CDecoder* loader, DecoderStatus status);

protected:
    QNetworkAccessManager   m_manager;
    std::vector<std::shared_ptr<CDecoder>>    m_decoders;

 private:
    static CDownloadManager* s_poInstance;
};

#define DownloadMgr (*CDownloadManager::getInstance())

#endif // CDOWNLOADMANAGER_H
