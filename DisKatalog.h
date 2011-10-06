#ifndef DISKATALOG_H
#define DISKATALOG_H

#include <QtGui>
#include <QtGui/qmainwindow.h>
#include <QtGui/qapplication.h>
#include <QtGui/qsplitter.h>
#include <QtGui/qaction.h>
#include <QtGui/qmenu.h>

#include "helper.h"

class DisKatalog : public QMainWindow
{
    Q_OBJECT

public:
    DisKatalog(QStringList args, QWidget * parent = 0, Qt::WindowFlags f = 0 );
private:
//    int dbconnum;
    QMenu *fileMenu;
    QMenu *editMenu;

    QAction *newAct;
    QAction *openAct;
    QAction *closeAct;
    QAction *exitAct;
    QAction *addMediaAct;
    QAction *deleteSelectedAct;
    QAction *testAct;
    QList<CatalogList> catList;

    void createActions();
    void createMenus(QMainWindow* parent);

    QTreeWidget* treeView;
    QTreeWidget* listView;

    QStringList lvCatalogHeaders;
    QStringList lvDefaultHeaders;

    void populateListView();
    bool openCatalog(const QString &fileName);

private slots:
    bool createCatalog();
    void openFile();
    void closeCatalog();
    bool addMedia();

    void listDb();
    void deleteSelected();

    void TVitemExpanded (QTreeWidgetItem * item);
//    void LVcurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous);
    void TVcurrentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous);
    void LVitemDoubleClick(QTreeWidgetItem* item, int column);
};


#endif // DISKATALOG_H
