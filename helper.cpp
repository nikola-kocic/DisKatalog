#include "helper.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdir.h>

#include <QtSql/qsqlerror.h>
#include <QtGui/qapplication.h>


QTreeWidgetItem* gettvri(QTreeWidgetItem* ctvi)
{
    while(ctvi->parent()!=NULL)
        ctvi = ctvi->parent();

    return ctvi;
}

QIcon getSystemIcon(const int type)
{
    QStyle::StandardPixmap sp;
    if(type == TYPE_DIR) sp = QStyle::SP_DirIcon;
    else if(type == TYPE_CATALOG) sp = QStyle::SP_DialogOpenButton;
    else if(type == TYPE_MEDIA) sp = QStyle::SP_DriveDVDIcon;
    else sp = QStyle::SP_FileIcon;

    return QApplication::style()->standardIcon(sp);
}

bool execQuery(QSqlQuery* query, const QString& qs)
{
    //qDebug() << QSqlDatabase::connectionNames();
    qDebug() << qs;
    if(!query->exec(qs))
    {
//        QMessageBox::critical(0, QObject::tr("query Error"), q->lastError().text());
        qDebug() << "query Error" << query->lastError().text();
        qDebug() << query->lastError().text();
        return false;
    }
    qDebug() << "success";
    return true;
}

bool createCatalog_helper(const QString& dbPath, const QString& dbname)
{
    qDebug() << "createCatalog" << dbPath;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", dbPath);
    db.setDatabaseName(dbPath);
    if(!db.open()) return false;

    QSqlQuery* query = new QSqlQuery(db);

    execQuery(query,("CREATE TABLE IF NOT EXISTS name (name TEXT);"));
    execQuery(query,("CREATE TABLE IF NOT EXISTS media (id INTEGER PRIMARY KEY AUTOINCREMENT, root_dir_id INTEGER, name TEXT, time DATE, size INTEGER, category TEXT);"));
    execQuery(query,("CREATE TABLE IF NOT EXISTS dirs (id INTEGER PRIMARY KEY AUTOINCREMENT, media_id INTEGER, parent_dir_id INTEGER, name TEXT, time DATE, size INTEGER, FOREIGN KEY (media_id) REFERENCES media(id) ON DELETE CASCADE);"));
    execQuery(query,("CREATE TABLE IF NOT EXISTS files (id INTEGER PRIMARY KEY AUTOINCREMENT, dir_id INTEGER, name TEXT, ext TEXT, time DATE, size INTEGER, FOREIGN KEY (dir_id) REFERENCES dirs(id) ON DELETE CASCADE);"));
    execQuery(query,("INSERT INTO name (name) VALUES ('" + dbname + "');"));
    db.close();

    return true;
}

//bool openCatalog_helper(const QString& dbPath, QTreeWidget* treeView)
//{
//    if(QSqlDatabase::contains(dbPath)) return false;

//    qDebug() << "open catalog" << dbPath;
//    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", dbPath);
//    db.setDatabaseName(dbPath);
//    if(!db.open()) return false;

//    QSqlQuery* query = new QSqlQuery(db);
//    execQuery(query,"SELECT name FROM name LIMIT 1");
//    query->next();

//    QTreeWidgetItem* ntvri = new QTreeWidgetItem();
//    ntvri->setText(TV_COL_NAME, query->value(0).toString());
//    db.close();

//    ntvri->setIcon(TV_COL_NAME, getSystemIcon("catalog"));
//    ntvri->setText(TV_COL_TYPE, "catalog");
//    ntvri->setText(TV_COL_DBPATH, dbPath);
//    ntvri->addChild(new QTreeWidgetItem());
//    treeView->addTopLevelItem(ntvri);

//    return true;
//}


bool openCatalog_helper(const QString &dbPath, QTreeWidget* treeView)
{
    if(QSqlDatabase::connectionNames().contains(dbPath))
    {
        return false;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", dbPath);
    db.setDatabaseName(dbPath);
    if(!db.open()) return false;

    QSqlQuery* query = new QSqlQuery(db);
    if(!execQuery(query,"SELECT name FROM 'name' LIMIT 1")) return false;

    QTreeWidgetItem* ntvri = new QTreeWidgetItem(TYPE_CATALOG);
    query->next();

    ntvri->setText(TV_COL_NAME, query->value(0).toString());
    ntvri->setIcon(TV_COL_NAME, getSystemIcon(ntvri->type()));
    ntvri->setText(TV_COL_DBPATH, dbPath);
    treeView->addTopLevelItem(ntvri);

    if(!execQuery(query,"SELECT id,name,root_dir_id FROM 'media'")) return false;

    while (query->next())
    {
        QTreeWidgetItem* root = new QTreeWidgetItem(TYPE_MEDIA);
        root->setIcon(TV_COL_NAME, getSystemIcon(root->type()));
        root->setText(TV_COL_NAME, query->value(1).toString());
        root->setText(TV_COL_DBPATH, query->value(2).toString());
        root->addChild(new QTreeWidgetItem());
        ntvri->addChild(root);
    }

    db.close();

    return true;
}

QList<QTreeWidgetItem*> getTVCatalogContent(const QString& dbPath)
{
    QList<QTreeWidgetItem*> items;

    QSqlDatabase db = QSqlDatabase::database(dbPath);
    QSqlQuery* query = new QSqlQuery(db);

    execQuery(query,"SELECT id, root_dir_id, name FROM media");

    while (query->next())
    {
        QTreeWidgetItem* ntvi = new QTreeWidgetItem(TYPE_MEDIA);
        ntvi->setIcon(TV_COL_NAME, getSystemIcon(ntvi->type()));
        ntvi->setText(TV_COL_NAME, query->value(2).toString());
        ntvi->setText(TV_COL_DBPATH, query->value(1).toString());
        ntvi->addChild(new QTreeWidgetItem());
        items << ntvi;
    }

    db.close();
    return items;
}

QList<QTreeWidgetItem*> getTVDirectoryContent(const QString& dbPath, const QString& parent_dir_id)
{
    QList<QTreeWidgetItem*> items;

    QSqlDatabase db = QSqlDatabase::database(dbPath);
    QSqlQuery* query = new QSqlQuery(db);

    execQuery(query, "SELECT id, name FROM dirs WHERE parent_dir_id='" + parent_dir_id + "'");

    while (query->next())
    {
        QTreeWidgetItem* ntvi = new QTreeWidgetItem(TYPE_DIR);
        ntvi->setText(TV_COL_NAME, query->value(1).toString());
        ntvi->setIcon(TV_COL_NAME, getSystemIcon(ntvi->type()));
        ntvi->setText(TV_COL_DBPATH, query->value(0).toString());
        ntvi->addChild(new QTreeWidgetItem(TYPE_DUMMY));
        items << ntvi;
    }

    db.close();
    return items;
}


QList<QTreeWidgetItem*> getLVCatalogContent(const QString& catpath)
{
    QList<QTreeWidgetItem*> items;

    QString queryString;

    QSqlDatabase db = QSqlDatabase::database(catpath);
    QSqlQuery* query = new QSqlQuery(db);

    queryString = "SELECT name, size, time, category FROM media";
    execQuery(query, queryString);
    while (query->next())
    {
        QTreeWidgetItem* nlvi = new QTreeWidgetItem(TYPE_MEDIA);
        nlvi->setText(LV_COL_MEDIA_NAME, query->value(0).toString());
        nlvi->setText(LV_COL_MEDIA_SIZE, QLocale().toString(query->value(1).toInt()));
        nlvi->setText(LV_COL_MEDIA_TIME, QDateTime::fromString(query->value(2).toString(), Qt::ISODate).toString(Qt::SystemLocaleShortDate));
        nlvi->setText(LV_COL_MEDIA_CAT, query->value(3).toString());
        nlvi->setIcon(LV_COL_MEDIA_NAME, getSystemIcon(nlvi->type()));
        items << nlvi;
    }

    db.close();
    return items;
}


QList<QTreeWidgetItem*> getLVDirectoryContent(const QString& dbPath, const QString& dir_id)
{
    QList<QTreeWidgetItem*> items;

    QString queryString;

    QSqlDatabase db = QSqlDatabase::database(dbPath);
    QSqlQuery* query = new QSqlQuery(db);


    queryString = "SELECT name, size, time FROM dirs WHERE parent_dir_id='" + dir_id + "'";

    execQuery(query, queryString);

    while (query->next())
    {
        QTreeWidgetItem* nlvi = new QTreeWidgetItem(TYPE_DIR);
        nlvi->setText(LV_COL_NAME, query->value(0).toString());
        nlvi->setText(LV_COL_SIZE, QLocale().toString(query->value(1).toInt()));
        nlvi->setText(LV_COL_TIME, QDateTime::fromString(query->value(2).toString(), Qt::ISODate).toString(Qt::SystemLocaleShortDate));
        nlvi->setIcon(LV_COL_NAME, getSystemIcon(nlvi->type()));
        items << nlvi;
    }

    queryString = "SELECT name, ext, size, time FROM files WHERE dir_id='" + dir_id + "'";
    execQuery(query, queryString);
    while (query->next())
    {
        QTreeWidgetItem* nlvi = new QTreeWidgetItem(TYPE_FILE);
        nlvi->setText(LV_COL_NAME, query->value(0).toString());
        nlvi->setText(LV_COL_EXT, query->value(1).toString());
        nlvi->setText(LV_COL_SIZE, QLocale().toString(query->value(2).toInt()));

        nlvi->setText(LV_COL_TIME, QDateTime::fromString(query->value(3).toString(), Qt::ISODate).toString(Qt::SystemLocaleShortDate));
        nlvi->setIcon(LV_COL_NAME, getSystemIcon(nlvi->type()));
        items << nlvi;
    }


    db.close();

    return items;
}

void addMedia_helper(const QString& mediaName, const QString& mediaPath, const QString& category, QTreeWidgetItem* ctvri)
{
    QFileInfo fi(mediaPath);
    QTreeWidgetItem* ntvri = new QTreeWidgetItem(ctvri, TYPE_MEDIA);
    ntvri->setIcon(TV_COL_NAME, getSystemIcon(ntvri->type()));
    ntvri->setText(TV_COL_NAME, mediaName);

    qDebug() << "appendMedia" << ctvri->text(TV_COL_DBPATH);
    QSqlDatabase db = QSqlDatabase::database(ctvri->text(TV_COL_DBPATH));
    QSqlQuery* query = new QSqlQuery(db);
    int mediaSum = 0;
    MediaInfo mi;

    execQuery(query,("BEGIN;"));
    execQuery(query,("INSERT INTO media (name, time, category)  VALUES ('" + mediaName + "', '" + fi.lastModified().toString(Qt::ISODate) + "', '" + category + "');"));
    mi.media_id = query->lastInsertId().toString();

    execQuery(query,("INSERT INTO dirs (media_id, parent_dir_id, name, time) VALUES ('" + mi.media_id + "','0', '/', '" + fi.lastModified().toString(Qt::ISODate) + "');"));
    mi.dir_id = query->lastInsertId().toString();

    execQuery(query,("UPDATE media SET root_dir_id='" + mi.dir_id + "' WHERE id=" + mi.media_id + ";"));

    ntvri->setText(TV_COL_DBPATH, mi.dir_id);
    RecurseDirectory(query, mediaPath, ntvri, mediaSum, &mi);

    execQuery(query,("UPDATE media SET size='" + QString::number(mediaSum) + "' WHERE id=" + mi.media_id + ";"));
    execQuery(query,("COMMIT;"));

    db.close();
}

void RecurseDirectory(QSqlQuery* query, const QString& sDir, QTreeWidgetItem* ctvi, int& parentSum,  MediaInfo* mi)
{
    QDir dir(sDir);
    QFileInfoList list = dir.entryInfoList();
    for (int i=0; i < list.count(); i++)
    {
        QFileInfo info = list[i];

        if (info.fileName()!=".." && info.fileName()!=".")
        {
            QString dir_id = mi->dir_id;
            if (info.isDir())
            {
                int dirSum = 0;
                QTreeWidgetItem* ntvi = new QTreeWidgetItem(ctvi, TYPE_DIR);
                ntvi->setIcon(TV_COL_NAME, getSystemIcon(ntvi->type()));
                ntvi->setText(TV_COL_NAME, info.fileName());

                execQuery(query,("INSERT INTO dirs (media_id, parent_dir_id, name, time) VALUES ('" + mi->media_id + "', '" + dir_id + "', '" + info.completeBaseName() + "', '" + info.lastModified().toString(Qt::ISODate) + "');"));
                mi->dir_id = query->lastInsertId().toString();

                ntvi->setText(TV_COL_DBPATH, mi->dir_id);
                RecurseDirectory(query, info.filePath(), ntvi, dirSum, mi); // recursive

                execQuery(query,("UPDATE dirs SET size='" + QString::number(dirSum) + "' WHERE id=" + mi->dir_id + ";"));
                mi->dir_id = dir_id;
                parentSum += dirSum;
            }
            else
            {
                execQuery(query,("INSERT INTO files (dir_id, name, ext, time, size) VALUES ('" +  dir_id + "', '" + info.completeBaseName() + "', '" + info.suffix() + "', '" + info.lastModified().toString(Qt::ISODate) + "', '" + QString::number(info.size()) + "');"));

                parentSum += info.size();
            }
        }
    }
}

void removeMedia_helper(QTreeWidgetItem* ctvi)
{
    QTreeWidgetItem* ctvri = gettvri(ctvi);

    QSqlDatabase db = QSqlDatabase::database(ctvri->text(TV_COL_DBPATH));
//    db.open();
    QSqlQuery* query = new QSqlQuery(db);
    execQuery(query, "PRAGMA foreign_keys = ON");
    execQuery(query, "DELETE FROM media WHERE name='"+ ctvi->text(TV_COL_NAME) +"';");
    db.close();

    delete ctvi;
}

void validMediaName(QString& mediaName, QTreeWidgetItem* ctvri)
{
    QString newMediaName = mediaName;
    int p=2;

    for(int i = 0; i < ctvri->childCount(); i++)
    {
        if(newMediaName == ctvri->child(i)->text(TV_COL_NAME))
        {
            newMediaName = mediaName + " (" + QString::number(p) + ")";
            p++;
            i = 0;
        }
    }

    mediaName = newMediaName;
}


int getIdFromTreewidget(int number)
{
    return number - 2000;
}

int makeIdForTreewidget(int number)
{
    return number + 2000;
}
