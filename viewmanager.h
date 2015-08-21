#ifndef CVIEWMANAGER_H
#define CVIEWMANAGER_H

#include <QObject>
#include <memory>  // make_shared
#include <vector>
#include "greatwindow.h"
#include "decoder.h"

class CViewManager: public QObject
{
    Q_OBJECT

public:
    explicit CViewManager(QObject *parent = 0);
     virtual ~CViewManager();

    void start();
    void show();
    void quit();
    void quitOne(const GreatWindow* pWnd);
    void activeNext(const GreatWindow* pWnd);
    void updateRecent();

    void present(const QString& url);
    GreatWindow *findWindow(QSize psz);
    void setDuplicate();

    static CViewManager* getInstance();

protected:
    bool    m_bSarted;
    std::vector<std::shared_ptr<GreatWindow>>   m_vWindows;
    bool    m_bDuplicated;

private:
    static CViewManager* s_poInstance;
};

#define ViewMgr (*CViewManager::getInstance())

#endif // CVIEWMANAGER_H
