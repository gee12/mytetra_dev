#ifndef _TAGSTABLEWIDGET_H_
#define	_TAGSTABLEWIDGET_H_

#include <QWidget>
#include "models/tags/TagsModel.h"

class QModelIndex;
class QTableView;
class QStandardItemModel;


class TagsTableWidget : public QWidget
{
  Q_OBJECT

public:
  TagsTableWidget(QWidget *parent=nullptr);
  virtual ~TagsTableWidget();

  void onDataLoaded();
  void clearAll();
  void setOverdrawMessage(QString message);

  QTableView* getTableView();

private:
  QTableView *tagsTableView;
  QString overdrawMessage;

  void setupUI();
  void setupSignals();
  void assembly();

  void paintEvent(QPaintEvent *event);
  void resizeEvent(QResizeEvent *event);
  void onSectionResized(int logicalIndex, int oldSize, int newSize);
};

#endif	/* _TAGSTABLEWIDGET_H_ */

