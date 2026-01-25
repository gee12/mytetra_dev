#include "main.h"

#include "models/tree/TreeItem.h"
#include "models/tags/TagsModel.h"
#include "models/appConfig/AppConfig.h"
#include "views/mainWindow/MainWindow.h"
#include "libraries/GlobalParameters.h"
#include "libraries/FixedParameters.h"

extern GlobalParameters globalParameters;
extern AppConfig mytetraConfig;


TagsModel::TagsModel(QObject *parent) : QStandardItemModel(parent)
{
    isUnsearchCryptBranchPresent = false;
}


TagsModel::~TagsModel()
{
}


void TagsModel::init(TreeItem *startNode)
{
    isUnsearchCryptBranchPresent = false;
    data.clear();

    beginResetModel();

    readNodeTagsRecursively(startNode);

    for (auto it = data.begin(); it != data.end(); ++it) {
        addRow(it.key(), it.value()->size());
    }

    endResetModel();
}


void TagsModel::readNodeTagsRecursively(TreeItem *node)
{
    // Если данная ветка - "Избранное", то пропускаем ее
    if (node->getField("id") == FixedParameters::favoritesItemId)
        return;

    // Если ветка зашифрована, и пароль не был введен
    if (node->getField("crypt")=="1" &&
        globalParameters.getCryptKey().length()==0)
    {
        isUnsearchCryptBranchPresent = true;
        return;
    }

    // Если у ветки есть записи, перебираем их
    if (node->recordtableGetRowCount() > 0)
    {
        RecordTableData *recordTable = node->recordtableGetTableData();
        for (int i=0; i<static_cast<int>(recordTable->size()); i++)
            readRecordTags(recordTable, i);
    }

    // Рекурсивная обработка каждой подчиненной ветки
    for (int i=0; i<node->childCount(); i++)
        readNodeTagsRecursively(node->child(i));
}


void TagsModel::readRecordTags(RecordTableData *searchRecordTable, int index)
{
    Record *record = searchRecordTable->getRecord(index);
    QString tagsString = searchRecordTable->getField("tags", index);

    // Разделяем строку на отдельные метки
    QStringList tagsList = tagsString.split(',', Qt::SkipEmptyParts);

    // Перебираем все метки
    foreach (const QString &tag, tagsList)
    {
        QString tagValue = tag.trimmed().toLower();

        if (data.contains(tagValue)) {
            // Если метка уже была добавлена, тогда данную заметку просто добавляем в список
            data[tagValue]->insertRecordByTag(record);
        } else {
            // Иначе добавляем метку с данной заметкой
            auto *tableData = new RecordTableData();
            tableData->insertRecordByTag(record);
            data[tagValue] = tableData;
        }
    }
}


void TagsModel::addRow(const QString &tagName, const int recordsCount)
{
    int index = rowCount();
    insertRow(index);

    // Имя метки
    auto *itemInfo = new QStandardItem();
    itemInfo->setText(tagName);

    // В ячейке заголовка хранится имя метки для дальнейшего использования
    // qDebug() << "Tag name " << tagName;
    itemInfo->setData(tagName, USER_ROLE_TAG_NAME);
    itemInfo->setData(tagName, SORT_ROLE);

    // Кол-во заметок
    auto *itemRecordCount = new QStandardItem();
    QString recordsCountString;
    itemRecordCount->setText(recordsCountString.setNum(recordsCount));
    itemRecordCount->setData(recordsCount, SORT_ROLE);

    setItem(index, 0, itemInfo);
    setItem(index, 1, itemRecordCount);
    setSortRole(SORT_ROLE);
}


QString TagsModel::getTagNameByIndex(const QModelIndex &index)
{
    QStandardItem *clickItem = itemFromIndex(index);
    int columnIndex = 0;
    QStandardItem *itemInfo = item(clickItem->row(), columnIndex);

    return itemInfo->data(USER_ROLE_TAG_NAME).toString();
}


RecordTableData* TagsModel::getRecordTableByIndex(const QModelIndex &index, TreeItem *startNode)
{
    QString tagName = getTagNameByIndex(index);
    qDebug() << "Get records list by tag:" << tagName;

    RecordTableData *tableData = data[tagName];

    // Заполняем таблицу заново (актуализируем)
    tableData->empty();
    QList<Record*> recordsByTag = findRecordsByTagRecursively(tagName, startNode);
    foreach (Record *record, recordsByTag)
        tableData->insertRecordByTag(record);

    return tableData;
}

QList<Record*> TagsModel::findRecordsByTagRecursively(const QString &tagName, TreeItem *node)
{
    QList<Record*> list;

    // Если данная ветка - "Избранное", то пропускаем ее
    if (node->getField("id") == FixedParameters::favoritesItemId)
        return list;

    // Если ветка зашифрована, и пароль не был введен
    if (node->getField("crypt")=="1" &&
        globalParameters.getCryptKey().length()==0)
    {
        isUnsearchCryptBranchPresent = true;
        return list;
    }

    // Если у ветки есть записи, перебираем их
    if (node->recordtableGetRowCount() > 0)
    {
        RecordTableData *recordTable = node->recordtableGetTableData();
        for (int i=0; i<static_cast<int>(recordTable->size()); i++)
        {
            Record *record = recordTable->getRecord(i);
            QString tagsString = recordTable->getField("tags", i);

            // Разделяем строку на отдельные метки
            QStringList tagsList = tagsString.split(',', Qt::SkipEmptyParts);

            // Перебираем все метки
            foreach (const QString &tag, tagsList)
            {
                // Если метка совпадает, добавляем данную заметку в список
                QString tagValue = tag.trimmed().toLower();
                if (tagName == tagValue)
                    list.append(record);
            }
        }
    }

    // Рекурсивная обработка каждой подчиненной ветки
    for (int i=0; i<node->childCount(); i++)
    {
        foreach (Record *record, findRecordsByTagRecursively(tagName, node->child(i)))
            list.append(record);
    }

    return list;
}
