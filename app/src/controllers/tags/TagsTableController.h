#ifndef __TAGSTABLECONTROLLER_H__
#define __TAGSTABLECONTROLLER_H__

#include <QObject>
#include <QTextDocument>

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
  void selectTag(const QModelIndex &sourceIndex) const;
  void selectTag(const QString &tagName) const;
  void findNextTag(const QString &text, QTextDocument::FindFlags flags);
  void renameTag(const QModelIndex &proxyIndex, QString newName);
  void deleteTag(QModelIndex proxyIndex);
  TagsTableWidget *getView();

public slots:
  void onSortChanged(int columnIndex, Qt::SortOrder order) const;
  void onTagClicked(const QModelIndex &proxyIndex) const;
  void onCopyTagReferenceContext();

protected:
  void sortTags();
  bool checkRowMatching(const QString &text, int row);

  TagsTableWidget *view;
  TagsModel *sourceModel;
  QSortFilterProxyModel *proxyModel;
};

#endif // __TAGSTABLECONTROLLER_H__
