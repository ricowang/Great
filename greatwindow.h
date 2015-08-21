#ifndef GREATWINDOW_H
#define GREATWINDOW_H

#include <QMainWindow>
#include <memory>
#include "decoder.h"
#include <QTimer>

namespace Ui {
class GreatWindow;
}

class CCanvas;

class GreatWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GreatWindow(int monitor, bool main, QWidget *parent = 0);
    ~GreatWindow();

    void closeEvent(QCloseEvent *event);
    void display(const QImage& img);
    void setLoader(std::shared_ptr<CDecoder> decoder);
    void setImageActSize(const QSize& sz);
    void updateRecent();

public slots:
    void loaderProgress(CDecoder* loader, DecoderStatus status);
    void openRecentFile();

protected:
    void keyPressEvent(QKeyEvent * event);
    void keyReleaseEvent(QKeyEvent * event);
    void wheelEvent(QWheelEvent * event);
    void reverFullScreen();
    void checkShift();

    bool m_jumping;
    QString m_mainUrl;
    QByteArray m_geoData;
    QAction* m_recentFileActions[5];

private slots:
    void on_action_Quit_triggered();
    void on_action_Open_triggered();

private:
    Ui::GreatWindow *ui;
    int             m_monitor;
    bool            m_bMainWindow;
    QPixmap         m_pixmap;
    CCanvas*        m_canvas;


    std::shared_ptr<CDecoder> m_decoder;
    std::vector<std::shared_ptr<CDecoder>> m_lasts;
    QTimer                    m_timer;
};

#endif // GREATWINDOW_H
