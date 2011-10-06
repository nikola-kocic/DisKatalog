#include "DisKatalog.h"

#include <QtCore/qdebug.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qfileinfo.h>

#include <QtGui/qmenubar.h>
#include <QtGui/qfiledialog.h>

/*
ctvri = Current TreeView Root Item
ctvi = Current TreeView Item
ntvri = New TreeView Root Item
ntvi = New TreeView Item
tvpi = TreeView Parent Item

nlvi = New ListView Item

etc.
*/

DisKatalog::DisKatalog (QStringList args, QWidget* parent, Qt::WindowFlags f)
{
    setWindowTitle(tr("DisKatalog"));
//    resize(800,400);
    createActions();
    createMenus(this);

    treeView = new QTreeWidget();
//    treeView->setHeaderHidden(true);
    treeView->setHeaderLabels(QStringList() << "Name" << "DBpath");

    listView = new QTreeWidget();
    listView->setRootIsDecorated(false);
    listView->sortByColumn(LV_COL_NAME, Qt::AscendingOrder);
    lvCatalogHeaders << "Name"  << "Size" << "Time" << "Added" << "Category";
    lvDefaultHeaders << "Name" << "Ext" << "Size" << "Time";
    listView->setHeaderLabels(lvCatalogHeaders);
    listView->setColumnWidth(LV_COL_NAME, 220);
    listView->setColumnWidth(LV_COL_EXT, 70);
    listView->setSelectionBehavior(QAbstractItemView::SelectRows);
    listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listView->setSortingEnabled(true);

    QSplitter* splitter = new QSplitter();
    splitter->addWidget(treeView);
    splitter->addWidget(listView);



    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    policy.setHorizontalStretch(1);
    policy.setVerticalStretch(0);
    listView->setSizePolicy(policy);

    treeView->resize(300, treeView->height());

    this->setCentralWidget(splitter);


    for (int i = 1; i < args.count(); ++i)
    {
        openCatalog(args[i]);
    }

    treeView->connect(treeView, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(TVcurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));

    treeView->connect(treeView, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(TVitemExpanded(QTreeWidgetItem*)));

    listView->connect(listView, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(LVitemDoubleClick(QTreeWidgetItem*,int)));
}

void DisKatalog::createActions()
{
    newAct = new QAction(tr("&New Catalog..."), this);
    newAct->setShortcut(tr("Ctrl+N"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(createCatalog()));

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));

    closeAct = new QAction(tr("&Close Catalog"), this);
    closeAct->setShortcut(tr("Ctrl+F4"));
    connect(closeAct, SIGNAL(triggered()), this, SLOT(closeCatalog()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Alt+F4"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    addMediaAct = new QAction(tr("Add &Disk..."), this);
    addMediaAct->setShortcut(tr("Ctrl+D"));
    connect(addMediaAct, SIGNAL(triggered()), this, SLOT(addMedia()));

    deleteSelectedAct = new QAction(tr("De&lete Selected"), this);
    connect(deleteSelectedAct, SIGNAL(triggered()), this, SLOT(deleteSelected()));

    testAct = new QAction(tr("&Test"), this);
    connect(testAct, SIGNAL(triggered()), this, SLOT(listDb()));
}

void DisKatalog::createMenus(QMainWindow* parent)
{
    fileMenu = new QMenu(tr("&File"), parent);
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(closeAct);
    fileMenu->addAction(exitAct);

    editMenu = new QMenu(tr("&Edit"), parent);
    editMenu->addAction(addMediaAct);
    editMenu->addAction(deleteSelectedAct);
    editMenu->addAction(testAct);

    parent->menuBar()->addMenu(fileMenu);
    parent->menuBar()->addMenu(editMenu);
}


void DisKatalog::TVitemExpanded (QTreeWidgetItem * item)
{
    if(item->childCount() > 0)
    {
        if(item->child(0)->type() == TYPE_DUMMY)
        {
            delete item->child(0);
            QList<QTreeWidgetItem*> items;
            if(item->type() == TYPE_CATALOG)
            {
                items = getTVCatalogContent(item->text(TV_COL_DBPATH));
            }
            else
            {
                items = getTVDirectoryContent(gettvri(item)->text(TV_COL_DBPATH), item->text(TV_COL_DBPATH));
            }

            item->addChildren(items);
        }
    }
}

void DisKatalog::LVitemDoubleClick(QTreeWidgetItem* item, int column)
{
    if(item->type() != TYPE_FILE)
    {
        for(int i = 0; i < treeView->currentItem()->childCount(); i++)
        {
            if(treeView->currentItem()->child(i)->text(TV_COL_NAME) == item->text(LV_COL_NAME))
            {
                treeView->currentItem()->setExpanded(true);
                treeView->setCurrentItem(treeView->currentItem()->child(i));
                break;
            }
        }
    }
}

bool DisKatalog::openCatalog(const QString &filePath)
{
    if(openCatalog_helper(filePath, treeView) == false)
    {
        for(int i = 0; i < treeView->topLevelItemCount(); i++)
        {
            if(treeView->topLevelItem(i)->text(TV_COL_DBPATH) == filePath)
            {
                treeView->setCurrentItem(treeView->topLevelItem(i));
                qDebug() << filePath << "already opened";
                break;
            }
        }

        return false;
    }
    else return true;
}

void DisKatalog::TVcurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous)
{
    if(current != NULL) populateListView();
}

void DisKatalog::populateListView()
{
    //        QTime t;
    //        t.start();

    QTreeWidgetItem* ctvi = treeView->currentItem();
    if(ctvi != NULL)
    {
        TVitemExpanded(ctvi);

        bool isCat = false;
        if(ctvi->type() == TYPE_CATALOG) isCat = true;

        QList<QTreeWidgetItem*> items;
        QString dbPath = gettvri(ctvi)->text(TV_COL_DBPATH);
        if(isCat)
        {
            items = getLVCatalogContent(dbPath);
        }
        else
        {
            items = getLVDirectoryContent(dbPath, ctvi->text(TV_COL_DBPATH));
        }

        listView->setUpdatesEnabled(false);
        listView->setSortingEnabled(false);

        listView->clear();

        if(isCat)
        {
            listView->setHeaderLabels(lvCatalogHeaders);
            listView->setColumnCount(lvCatalogHeaders.count());
        }
        else
        {
            listView->setHeaderLabels(lvDefaultHeaders);
            listView->setColumnCount(lvDefaultHeaders.count());
        }

        listView->addTopLevelItems(items);

        for(int i = 0; i < listView->headerItem()->columnCount(); i++)
        {
            listView->resizeColumnToContents(i);
        }

        listView->setSortingEnabled(true);
        listView->setUpdatesEnabled(true);
    }
    else
    {
        listView->clear();
    }
}

bool DisKatalog::createCatalog()
{
    QString filename = QFileDialog::getSaveFileName(this, "Location of New Catalog", "", tr("sqlite (*.sqlite);; All files (*.*)"));
    if (filename.isEmpty())
        return false;

    QString dbname = QFileInfo(filename).completeBaseName();

    if(createCatalog_helper(filename, dbname) == true)
    {
        QTreeWidgetItem* ntvri = new QTreeWidgetItem(TYPE_CATALOG);
        ntvri->setText(TV_COL_NAME, dbname);
        ntvri->setIcon(TV_COL_NAME, getSystemIcon(ntvri->type()));
        ntvri->setText(TV_COL_DBPATH, filename);
        treeView->addTopLevelItem(ntvri);
        return true;
    }
    else return false;
}

void DisKatalog::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Catalog", "", tr("sqlite (*.sqlite);; All files (*.*)"));
    if (!fileName.isEmpty())
        openCatalog(fileName);

}

void DisKatalog::deleteSelected()
{
    QTreeWidgetItem* ctvi = treeView->currentItem();

    if(listView->hasFocus() && ctvi->type() == TYPE_CATALOG)
    {
        bool update = false;
        foreach(QTreeWidgetItem* lvi, listView->selectedItems())
        {
            QTreeWidgetItem* ttvi = ctvi;
            for(int i = 0; i < ctvi->childCount(); i++)
            {
                ttvi = ctvi->child(i);
                if(ttvi->text(TV_COL_NAME)==lvi->text(LV_COL_NAME))
                {
                    removeMedia_helper(ttvi);
                    update = true;
                    break;
                }
            }
        }

        if(update == true) populateListView();
    }

    if(treeView->hasFocus())
    {
        if(ctvi->type() == TYPE_CATALOG) closeCatalog();
        else if(ctvi->type() == TYPE_MEDIA)
        {
            removeMedia_helper(treeView->currentItem());
            populateListView();
        }
    }
}

void DisKatalog::closeCatalog()
{
    QTreeWidgetItem* ctvri = gettvri(treeView->currentItem());

    QSqlDatabase::database(ctvri->text(TV_COL_DBPATH)).close();
    QSqlDatabase::removeDatabase(ctvri->text(TV_COL_DBPATH));

    if(treeView->topLevelItemCount() > 1) delete ctvri;
    else treeView->clear();

    listView->clear();
}

bool DisKatalog::addMedia()
{
    //            qDebug() << dbList.count();

    //            foreach (const QString &str, dbList.keys())
    //                qDebug() << str;

    //            for(int i=0; i<treeView->topLevelItemCount(); i++)
    //                qDebug() << treeView->topLevelItem(i)->text(TV_COLDBNAME);


    //        QDir dir(dirPath);
    //        dir.drives();

    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Disk");
    if (dirPath.isEmpty()) return false;

    QString mediaName = QFileInfo(dirPath).fileName();
    QTreeWidgetItem* ctvri = gettvri(treeView->currentItem());

    validMediaName(mediaName, ctvri);
    addMedia_helper(mediaName, dirPath, "", ctvri);
    populateListView();

    return true;
}

void DisKatalog::listDb()
{
    qDebug() << QSqlDatabase::connectionNames();
//    QTreeWidgetItem* ctvri = gettvri(treeView->currentItem());
//    QSqlDatabase db = QSqlDatabase::database(ctvri->text(TV_COL_DBPATH));
//    QSqlQuery* query = new QSqlQuery(db);

//    execQuery(query, "SELECT id, name, size, time, category FROM media");
//    while (query->next())
//    {
//        qDebug() << query->value(0).toString() << query->value(1).toString() << query->value(2).toString() << query->value(3).toString() << query->value(4).toString();
//    }

//    execQuery(query, "SELECT id, media_id, parent_dir_id, name, size, time FROM dirs");
//    while (query->next())
//    {

//        qDebug() << query->value(0).toString() << query->value(1).toString() << query->value(2).toString() << query->value(3).toString() << query->value(4).toString() << query->value(5).toString();
//    }

//    execQuery(query, "SELECT id, dir_id, name, ext, size, time FROM files");
//    while (query->next())
//    {
//        qDebug() << query->value(0).toString() << query->value(1).toString() << query->value(2).toString() << query->value(3).toString() << query->value(4).toString() << query->value(5).toString();
//    }


//    db.close();
}
