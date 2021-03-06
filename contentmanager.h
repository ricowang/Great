#ifndef CCONTENTMANAGER_H
#define CCONTENTMANAGER_H

#include <QString>
#include <vector>
#include <QWidget>

class CContentManager
{
 public:
    CContentManager();
    virtual ~CContentManager();

    bool open(QWidget *parent);
    void close();
    bool favorite(const QString& url);
    QString getUrl(int offset = 0, bool move = false, bool group = false);
    bool openFile(const QString& path);
    void addRecentFiles(const QString& path);
    void loadRecentFiles();

    QString getRecent(int idx);

public:
    static CContentManager* getInstance();

protected:
    std::vector<QString>    m_vUrls;
    int                     m_nCurrent;
    std::vector<QString>    m_recent;

 private:
    static CContentManager* s_poInstance;
};

#define ContentMgr (*CContentManager::getInstance())
#endif // CCONTENTMANAGER_H
