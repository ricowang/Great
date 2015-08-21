#include "decoder.h"
#include <QMutexLocker>
#include "viewmanager.h"
#include "downloadmanager.h"

#define locked(var) QMutexLocker var(&m_lock);

CDecoder::CDecoder(std::shared_ptr<CLoader> stream, QObject *parent):
    QThread(parent), m_stream(stream), m_status(DS_NotStarted),
    m_size(QSize(0, 0)), m_scanline(0), m_buffer(nullptr)
{
    qRegisterMetaType<DecoderStatus>("DecoderStatus");

    connect(this, &QThread::started, this, &CDecoder::work);
    connect(this, &CDecoder::statusChange,
            &DownloadMgr, &CDownloadManager::loaderProgress);

    this->moveToThread(this);
}

CDecoder::~CDecoder()
{
     if(nullptr != m_buffer) {
         free(m_buffer);
     }
}

QSize CDecoder::fullSize()
{
    if(m_status >= DS_HasHeader) {
        return m_size;
    }

    locked(lock);
    return m_size;
}

QSize CDecoder::size()
{
    if(m_status == DS_Finished || m_status == DS_Aborted) {
        return m_size;
    }

    locked(lock);
    return QSize(m_size.width(), m_scanline);
}

QImage CDecoder::image()
{
    if(m_status == DS_Decoding || m_status == DS_Finished)
    {
        int w = m_size.width();
        int rw = (int(w/4)*4) + (w%4?4:0);

        locked(lock);
        return QImage(m_buffer, rw, m_size.height(), QImage::Format_RGB888);
    }

    return QImage();
}

QString CDecoder::url()
{
    locked(lock);
    return m_stream->getUrl();
}

DecoderStatus CDecoder::status()
{
    locked(lock);
    return m_status;
}

bool CDecoder::SetStatus(DecoderStatus code)
{
    m_status = code;
    emit statusChange(this, code);
    if(DS_Aborted == m_status || DS_Finished == m_status)
    {
        return false;
    }

    return true;
}

#define SSTE(code) if(!SetStatus(code)) return;

void CDecoder::work()
{
    SSTE(DS_Started);

    // initialize
    m_dcinfo.err = jpeg_std_error(&m_jerr);
    jpeg_create_decompress(&m_dcinfo);

    m_dcinfo.src = (jpeg_source_mgr*)m_stream.get();

    // start decoding header
    int res = 0;
    res = jpeg_read_header(&m_dcinfo, TRUE);
    if (JPEG_HEADER_OK != res)
    {
        SSTE(DS_Aborted);
    }

    if(!jpeg_start_decompress(&m_dcinfo)) {
        SSTE(DS_Aborted);
    }

    int w = m_dcinfo.output_width;
    int rw = (int(w/4)*4) + (w%4?4:0);

    {
        locked(lock);
        m_size = QSize(m_dcinfo.output_width, m_dcinfo.output_height);
    }
    SSTE(DS_HasHeader);

    auto row_stride =  rw * m_dcinfo.output_components;
    auto buff_size = row_stride * m_dcinfo.output_height;
    m_buffer = (unsigned char*) malloc(buff_size);
    memset(m_buffer, 0, buff_size);

    auto jbuffer = (*m_dcinfo.mem->alloc_sarray)
                    ((j_common_ptr) &m_dcinfo, JPOOL_IMAGE, row_stride, 1);
    {
        locked(lock);
        SSTE((nullptr == m_buffer)?DS_Aborted:DS_Decoding);
    }

    while (m_dcinfo.output_scanline < m_dcinfo.output_height) {
        auto start = m_dcinfo.output_scanline;

        (void) jpeg_read_scanlines(&m_dcinfo, jbuffer, 1);

        auto diff = m_dcinfo.output_scanline - start;
        memcpy(m_buffer + (start * row_stride), *jbuffer, diff * row_stride );

        {
            locked(lock);
            m_scanline = m_dcinfo.output_scanline;
        }
        emit newScanline(this, m_dcinfo.output_scanline, m_dcinfo.output_height, diff);
    }

    jpeg_finish_decompress(&m_dcinfo);
    jpeg_destroy_decompress(&m_dcinfo);

    {
        locked(lock);
        m_status = DS_Finished;
    }
    quit();
    emit statusChange(this, m_status);
}

