#ifndef __TAGSTABLECONTROLLER_H__
#define __TAGSTABLECONTROLLER_H__

#include <QObject>

class QSortFilterProxyModel;
class TagsTableWidget;
class TagsModel;

class TagsTableController : public QObject
{
  Q_OBJECT

public:
  TagsTableController(QObject *parent = nullptr);
  virtual ~TagsTableController();

  void loadTags();
  void clearData();
  void clearSelection() const;
  void onTagClicked(const QModelIndex &proxyIndex) const;
  void onSortChanged(int columnIndex, Qt::SortOrder order) const;
  void findInTags();
  TagsTableWidget *getView();

  void renameTag(const QModelIndex &proxyIndex, QString newName);
  void deleteTag(QModelIndex proxyIndex);

public slots:
  void onCopyTagReferenceContext();

protected:
  void sortTags();

  TagsTableWidget *view;
  TagsModel *sourceModel;
  QSortFilterProxyModel *proxyModel;
};

#endif // __TAGSTABLECONTROLLER_H__
