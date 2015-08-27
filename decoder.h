#ifndef CDECODER_H
#define CDECODER_H

#include <QThread>
#include <QMutex>
#include <QSize>
#include <QImage>
#include <memory>
#include "jpeglib.h"
#include "loader.h"
#include <memory>

enum DecoderStatus {
    DS_NotStarted,  // not started, no header
    DS_Started,
    DS_HasHeader,   // header ready
    DS_Decoding,
    DS_Finished,    // success
    DS_Aborted,      // error
};

class CDecoder : public QThread
{
    Q_OBJECT

private:
    bool SetStatus(DecoderStatus code);

public:
    CDecoder(std::shared_ptr<CLoader> stream, QObject *parent = 0);
    ~CDecoder();
    QSize fullSize();
    QSize size();
    QImage image();
    QString url();
    DecoderStatus status();
    QString statusText();

signals:
    void statusChange(CDecoder* loader, DecoderStatus status);
    void newScanline(CDecoder* loader, int at, int total, int delta);

public slots:
    void work();

protected:
    std::shared_ptr<CLoader> m_stream;
    jpeg_decompress_struct  m_dcinfo;
    jpeg_error_mgr          m_jerr;
    DecoderStatus   m_status;
    QSize           m_size;
    int             m_scanline;
    QMutex          m_lock;
    unsigned char * m_buffer;
};


Q_DECLARE_METATYPE(DecoderStatus)

#endif // CDECODER_H
