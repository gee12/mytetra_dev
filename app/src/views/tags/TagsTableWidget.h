#ifndef _TAGSTABLEWIDGET_H_
#define	_TAGSTABLEWIDGET_H_

#include <QMenu>
#include <QWidget>

#include "controllers/tags/TagsTableController.h"
#include "models/tags/TagsModel.h"

class QModelIndex;
class QTableView;
class QStandardItemModel;


class TagsTableWidget : public QWidget
{
  Q_OBJECT

public:
  TagsTableWidget(QWidget *parent=nullptr, TagsTableController *controller=nullptr);
  virtual ~TagsTableWidget();

  void setController(TagsTableController *controller);
  void onDataLoaded();
  void clearAll();
  void setOverdrawMessage(QString message);
  void selectTableRow(const QModelIndex &proxyIndex);
  QModelIndex getFirstSelectedIndex();
  void setSectionsSizes(QList<int> sizes);
  QList<int> getSectionsSizes();
  QTableView* getTableView();
  int getVerticalScrollBarWidth();

protected slots:
  void onSectionResized(int logicalIndex, int oldSize, int newSize);
  void onCustomContextMenuRequested(const QPoint &mousePos);
  void onRenameTagContext();
  void onDeleteTagContext();

private:
  TagsTableController *controller;

  QMenu *contextMenu;

  QTableView *tagsTableView;
  QString overdrawMessage;

  QAction *actionRenameTag;
  QAction *actionCopyTagReference;
  QAction *actionDeleteTag;

  void setupUI();
  void setupActions();
  void setupSignals();
  void assemblyContextMenu();
  void assembly();

  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
};

#endif	/* _TAGSTABLEWIDGET_H_ */

