#ifndef CLOADER_H
#define CLOADER_H

#include <QObject>
#include <QNetworkReply>
#include "jpeglib.h"
#include <QFile>
#include <QMutex>

class CLoader : public jpeg_source_mgr
{
public:
    explicit CLoader(const QString& url);
    ~CLoader();
    QString getUrl() { return m_url; }

    virtual void init_source (j_decompress_ptr cinfo) = 0;
    virtual boolean fill_input_buffer(j_decompress_ptr cinfo) = 0;
    virtual void skip_input_data(j_decompress_ptr cinfo, long bytes) = 0;
    virtual boolean resync_to_restart(j_decompress_ptr cinfo, int desired)  = 0;
    virtual void term_source(j_decompress_ptr cinfo) = 0;

protected:
    QString     m_url;
};

class CNetworkLoader : public QObject, public CLoader
{
    Q_OBJECT;

public:
    explicit CNetworkLoader(const QString& url);
    ~CNetworkLoader();

    void init_source (j_decompress_ptr cinfo);
    boolean fill_input_buffer(j_decompress_ptr cinfo);
    void skip_input_data(j_decompress_ptr cinfo, long bytes);
    boolean resync_to_restart(j_decompress_ptr cinfo, int desired);
    void term_source(j_decompress_ptr cinfo);

protected:
    QNetworkReply* m_networkReply;
    QNetworkAccessManager m_networkManager;

private slots:
//    void onReady();
//    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onFinished(QNetworkReply*);
//    void onError(QNetworkReply::NetworkError);

private:
    JOCTET      *m_buffer;
    int         m_nLenght;
    QByteArray  m_byteArray;
    QMutex      m_lock;
};


#define FLOADER_BUFFER 1024*10
class CFileLoader : public CLoader
{
public:
    explicit CFileLoader(const QString& path);
    ~CFileLoader();

    void init_source (j_decompress_ptr cinfo);
    boolean fill_input_buffer(j_decompress_ptr cinfo);
    void skip_input_data(j_decompress_ptr cinfo, long bytes);
    boolean resync_to_restart(j_decompress_ptr cinfo, int desired);
    void term_source(j_decompress_ptr cinfo);

protected:
    QFile  m_file;
    JOCTET *m_buffer;

public slots:
};

#endif // CLOADER_H
