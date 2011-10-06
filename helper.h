#ifndef HELPER_H
#define HELPER_H

#include <QtSql/qsqlquery.h>
#include <QtSql/qsqldatabase.h>

#include <QtGui/qicon.h>
#include <QtGui/qtreewidget.h>

class MediaInfo
{
public:
    QString media_id;
    QString dir_id;
};

class TypeId
{
public:
    int type;
    int id;
};

class CatalogList
{
public:
    QList<TypeId> ti;
    QString path;
};

const int TV_COL_NAME = 0;
const int TV_COL_DBPATH = 1;

const int LV_COL_NAME = 0;
const int LV_COL_EXT = 1;
const int LV_COL_SIZE = 2;
const int LV_COL_TIME = 3;

const int LV_COL_MEDIA_NAME = 0;
const int LV_COL_MEDIA_SIZE = 1;
const int LV_COL_MEDIA_TIME = 2;
const int LV_COL_MEDIA_ADDED = 3;
const int LV_COL_MEDIA_CAT = 4;

const int TYPE_FILE = 1001;
const int TYPE_DIR = 1002;
//const int LV_TYPE_ARCHIVE = 1003;
const int TYPE_MEDIA = 1004;
const int TYPE_CATALOG = 1005;
const int TYPE_DUMMY = 1006;

bool execQuery(QSqlQuery* q, const QString& qs);
bool createCatalog_helper(const QString& filePath, const QString& dbname);
bool openCatalog_helper(const QString& dbPath, QTreeWidget* treeView);

void addMedia_helper(const QString& mediaName, const QString& mediaPath, const QString& category, QTreeWidgetItem* catalogTreeItem);
void RecurseDirectory(QSqlQuery* query, const QString& sDir, QTreeWidgetItem* ctvi, int& parentSum,  MediaInfo* mi);

QList<QTreeWidgetItem*> getLVCatalogContent(const QString& catpath);
//QList<QTreeWidgetItem*> getLVMediaContent(const QString& dbPath, const QString& media_id);
QList<QTreeWidgetItem*> getLVDirectoryContent(const QString& dbPath, const QString& dir_id);

//QString getMediaRootDir(QSqlQuery* query, const QString& media_id);
QList<QTreeWidgetItem*> getTVCatalogContent(const QString& dbPath);
//QList<QTreeWidgetItem*> getTVMediaContentHelper(const QString& dbPath, const QString& media_id);
QList<QTreeWidgetItem*> getTVDirectoryContent(const QString& dbPath, const QString& dir_id);

void removeMedia_helper(QTreeWidgetItem* ctvri);
void validMediaName(QString& mediaName, QTreeWidgetItem* treeView);

QTreeWidgetItem* gettvri(QTreeWidgetItem* ctvi);
QIcon getSystemIcon(const int type);


//First number is TYPE, rest is ID in sqlite
int getIdFromTreewidget(int number);
int makeIdForTreewidget(int number);

#endif // HELPER_H
