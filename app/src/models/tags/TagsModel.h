#ifndef __TAGSMODEL_H__
#define __TAGSMODEL_H__

#include <QStandardItemModel>
#include <QObject>
#include "models/recordTable/RecordTableData.h"
#include "models/recordTable/Record.h"

#define USER_ROLE_TAG_NAME      Qt::UserRole
#define SORT_ROLE               Qt::UserRole+1

class TreeItem;

class TagsModel : public QStandardItemModel
{

public:
    TagsModel(QObject *pobj=nullptr);
    ~TagsModel();

    void init(TreeItem *startNode);
    QString getTagNameByIndex(const QModelIndex & index);
    RecordTableData* getRecordTableByIndex(const QModelIndex &index, TreeItem *startNode);

    bool isUnsearchCryptBranchPresent; // были ли зашированные ветки, но пароль небыл введен

private:
    void readNodeTagsRecursively(TreeItem *node);
    void readRecordTags(RecordTableData *searchRecordTable, int index);
    void addRow(const QString &tagName, int recordsCount);
    QList<Record*> findRecordsByTagRecursively(const QString &tagName, TreeItem *startNode);

    QMap<QString, RecordTableData*> data;
};

#endif // __TAGSMODEL_H__
