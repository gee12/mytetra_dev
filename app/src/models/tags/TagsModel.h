#ifndef __TAGSMODEL_H__
#define __TAGSMODEL_H__

#include <QStandardItemModel>
#include <QObject>
#include "models/recordTable/RecordTableData.h"
#include "models/recordTable/Record.h"

#define USER_ROLE_TAG_NAME      Qt::UserRole
#define SORT_ROLE               Qt::UserRole+1

class TreeItem;

const QString TAG_SEPARATORS = "[,;]+";

class TagsModel : public QStandardItemModel
{

public:
    TagsModel(QObject *pobj=nullptr);
    ~TagsModel();

    void init(const TreeItem *startNode);
    QString getTagNameByIndex(const QModelIndex & index);
    QModelIndex getIndexByTagName(const QString &tagName);
    RecordTableData* getRecordTableByIndex(const QModelIndex &index, const TreeItem *startNode);
    QModelIndex renameTag(const QModelIndex &sourceIndex, const QString &newName);
    void deleteTag(const QModelIndex &sourceIndex);

    bool isUnsearchCryptBranchPresent; // были ли зашированные ветки, но пароль небыл введен

private:
    void readNodeTagsRecursively(const TreeItem *node);
    void readRecordTags(const RecordTableData *searchRecordTable, int index);
    void addRow(const QString &tagName, int recordsCount);
    QList<Record*> findRecordsByTagRecursively(const QString &tagName, const TreeItem *startNode);

    QMap<QString, RecordTableData*> data;
};

#endif // __TAGSMODEL_H__
