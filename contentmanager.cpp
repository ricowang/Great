#include "contentmanager.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QTextStream>
#include <algorithm>
#include "viewmanager.h"
#include <QSettings>

#define tr(x) (x)

CContentManager* CContentManager::s_poInstance = nullptr;

CContentManager* CContentManager::getInstance()
{
    if(nullptr == s_poInstance) {
       s_poInstance = new CContentManager();
    }

    return s_poInstance;
}

CContentManager::CContentManager()
{

}

CContentManager::~CContentManager()
{

}

bool CContentManager::open(QWidget* parent)
{
    QFileDialog dialog(parent);
    dialog.setModal(true);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("Image list (*.plx)");
    dialog.setWindowTitle("Open File");
    if(!dialog.exec()) {
        return false;
    }

    QString fileName =  dialog.selectedFiles().first();
    return openFile(fileName);
}

bool CContentManager::openFile(const QString& path)
{
    QFileInfo fi(path);

    if(!fi.exists() || !fi.isFile()) {
        return false;
    }

    close();

    QFile inputFile(path);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QTextStream in(&inputFile);
    while (!in.atEnd())
    {
        m_vUrls.push_back(in.readLine());
    }
    inputFile.close();

    std::sort(m_vUrls.begin(), m_vUrls.end());
    m_nCurrent = 0;

    addRecentFiles(path);

    return true;
}

void CContentManager::addRecentFiles(const QString& path)
{
    auto it = std::remove(m_recent.begin(), m_recent.end(), path);
    m_recent.erase(it, m_recent.end());
    m_recent.push_back(path);

    it = std::remove_if(m_recent.begin(), m_recent.end(), [](QString p){
        QFileInfo fi(p);
        return !fi.exists() || !fi.isFile();
    });
    m_recent.erase(it, m_recent.end());

    QStringList rfiles;
    for(auto it = m_recent.begin(); it != m_recent.end(); it++) {
        rfiles.append(*it);
    }
    QSettings rf("Great", "Great");
    rf.setValue("recen_files", rfiles);

    ViewMgr.updateRecent();
}

void CContentManager::loadRecentFiles()
{
    m_recent.clear();
    QSettings rf("Great", "Great");
    QStringList rfiles = rf.value("recen_files").toStringList();
    for(int i=0; i<rfiles.size(); i++){
        m_recent.push_back(rfiles.at(i));
    }
}

QString CContentManager::getRecent(int idx)
{
    if(idx >= m_recent.size()) {
        return "";
    }

    return m_recent[idx];
}

void CContentManager::close()
{
    m_vUrls.clear();
    m_nCurrent = 0;
}

bool CContentManager::favorite(const QString &url)
{
    QFile outfile("~/gf.plx");
    if (!outfile.open(QIODevice::Append)) {
        return false;
    }

    QTextStream out(&outfile);
    out << url << "\n";

    outfile.close();
}

QString CContentManager::getUrl(int offset, bool move, bool group)
{
    if(m_vUrls.size() == 0) {
        return "";
    }

    if(offset == 0) {
        return m_vUrls[m_nCurrent];
    }

    auto current = m_nCurrent;

    QString startUrl = m_vUrls[current];
    auto nLen = startUrl.lastIndexOf("/");
    startUrl = startUrl.left(nLen);

    bool ab = false;
    bool ae = false;

    while(offset != 0) {
        current += (offset>0)?1:-1;

        if(current == m_vUrls.size()){
            current = 0;
            if(ab) {
                return "";
            }
            ab = true;
        }
        if(current == -1){
            current = m_vUrls.size() - 1;
            if(ae) {
                return "";
            }
            ae = true;
        }

        if(!group) {
            offset += (offset>0)?-1:1;
        } else {
            auto thisUrl = m_vUrls[current].left(nLen);
            if(thisUrl != startUrl) {
                offset += (offset>0)?-1:1;
                startUrl = thisUrl;
            }
        }
    }

    if(move) {
        m_nCurrent = current;
    }

    return m_vUrls[current];
}

