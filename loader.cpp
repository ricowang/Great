#include "loader.h"
#define locked(var) QMutexLocker var(&m_lock);

void loader_init_source(j_decompress_ptr cinfo)
{
    auto loader = (CLoader*)cinfo->src;
    if(loader)
    {
        loader -> init_source(cinfo);
    }
}

boolean loader_fill_input_buffer(j_decompress_ptr cinfo)
{
    auto loader = (CLoader*)cinfo->src;
    if(loader)
    {
        return loader -> fill_input_buffer(cinfo);
    }
}

void loader_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
    auto loader = (CLoader*)cinfo->src;
    if(loader)
    {
        loader -> skip_input_data(cinfo, num_bytes);
    }
}

boolean loader_resync_to_restart(j_decompress_ptr cinfo, int desired)
{
    auto loader = (CLoader*)cinfo->src;
    if(loader)
    {
        loader -> resync_to_restart(cinfo, desired);
    }
}

void loader_term_source(j_decompress_ptr cinfo)
{
    auto loader = (CLoader*)cinfo->src;
    if(loader)
    {
        loader -> term_source(cinfo);
    }
}


CLoader::CLoader(const QString &url)
    : m_url(url)
{
    jpeg_source_mgr::next_input_byte = 0;
    jpeg_source_mgr::bytes_in_buffer = 0;
    jpeg_source_mgr::init_source = ::loader_init_source;
    jpeg_source_mgr::fill_input_buffer = ::loader_fill_input_buffer;
    jpeg_source_mgr::resync_to_restart = ::loader_resync_to_restart;
    jpeg_source_mgr::skip_input_data = :: loader_skip_input_data;
    jpeg_source_mgr::term_source = ::loader_term_source;
}

CLoader::~CLoader()
{
}

CFileLoader::CFileLoader(const QString &path):
    CLoader(path), m_file(path)
{
    m_buffer = new JOCTET[FLOADER_BUFFER];
    memset(m_buffer, 0, FLOADER_BUFFER);
}

CFileLoader::~CFileLoader()
{
    delete m_buffer;
}

void CFileLoader::init_source(j_decompress_ptr)
{
    m_file.open(QIODevice::ReadOnly);
}

boolean CFileLoader::fill_input_buffer(j_decompress_ptr cinfo)
{
    cinfo->src->next_input_byte = m_buffer;
    cinfo->src->bytes_in_buffer = m_file.read((char*)m_buffer, FLOADER_BUFFER);
    return cinfo->src->bytes_in_buffer > 0;
}

void CFileLoader::skip_input_data(j_decompress_ptr cinfo, long bytes)
{
    if(cinfo->src->bytes_in_buffer>bytes)
    {
        cinfo->src->next_input_byte += bytes;
        cinfo->src->bytes_in_buffer -= bytes;
    }
    else
    {
        m_file.seek(cinfo->src->bytes_in_buffer - bytes);

        cinfo->src->next_input_byte  = nullptr;
        cinfo->src->bytes_in_buffer  = 0;
    }
}

boolean CFileLoader::resync_to_restart(j_decompress_ptr cinfo, int desired)
{
    return jpeg_resync_to_restart(cinfo, desired);
}

void CFileLoader::term_source(j_decompress_ptr)
{
    m_file.close();
}

CNetworkLoader::CNetworkLoader(const QString &url):
    CLoader(url), QObject(), m_buffer(nullptr)
{
    m_lock.lock();

     QNetworkRequest request(url);
     m_networkReply = m_networkManager.get(request);

//     connect(m_networkReply, &QNetworkReply::downloadProgress,
//             this, &CNetworkLoader::onDownloadProgress);

     connect(&m_networkManager, &QNetworkAccessManager::finished,
             this, &CNetworkLoader::onFinished);
//     connect(m_networkReply, SIGNAL(error(QNetworkReply::NetworkError)),
     //             this, SLOT(onError(QNetworkReply::NetworkError)));
}

CNetworkLoader::~CNetworkLoader()
{

}

void CNetworkLoader::init_source(j_decompress_ptr cinfo)
{
    m_lock.lock();
}

boolean CNetworkLoader::fill_input_buffer(j_decompress_ptr cinfo)
{
    cinfo->src->next_input_byte = m_buffer;
    cinfo->src->bytes_in_buffer = m_byteArray.size();
    return cinfo->src->bytes_in_buffer > 0;

}

void CNetworkLoader::skip_input_data(j_decompress_ptr cinfo, long bytes)
{
    if(cinfo->src->bytes_in_buffer>bytes)
    {
        cinfo->src->next_input_byte += bytes;
        cinfo->src->bytes_in_buffer -= bytes;
    } else
    {
        Q_ASSERT(0);
    }
}

boolean CNetworkLoader::resync_to_restart(j_decompress_ptr cinfo, int desired)
{
    return jpeg_resync_to_restart(cinfo, desired);
}

void CNetworkLoader::term_source(j_decompress_ptr cinfo)
{

}

void CNetworkLoader::onFinished(QNetworkReply * reply)
{
    if (!reply->error()) {
        if (reply->open(QIODevice::ReadOnly)) {
            m_byteArray = m_networkReply->readAll();
            m_buffer = (JOCTET*) m_byteArray.data();
            m_nLenght = m_byteArray.length();
            reply->close();
        }
    }

    reply->deleteLater();
    m_lock.unlock();
}
