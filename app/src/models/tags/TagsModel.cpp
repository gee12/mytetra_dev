#include "main.h"

#include "models/tree/TreeItem.h"
#include "models/tags/TagsModel.h"
#include "models/appConfig/AppConfig.h"
#include "views/mainWindow/MainWindow.h"
#include "libraries/GlobalParameters.h"
#include "libraries/FixedParameters.h"

extern GlobalParameters globalParameters;
extern AppConfig mytetraConfig;

TagsModel::TagsModel(QObject *parent) : QStandardItemModel(parent) {
    isUnsearchCryptBranchPresent = false;
}


TagsModel::~TagsModel() {
}


void TagsModel::init(const TreeItem *startNode) {
    isUnsearchCryptBranchPresent = false;
    data.clear();

    beginResetModel();

    readNodeTagsRecursively(startNode);

    for (auto it = data.begin(); it != data.end(); ++it) {
        addRow(it.key(), it.value()->size());
    }

    endResetModel();
}


void TagsModel::readNodeTagsRecursively(const TreeItem *node) {
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
    if (node->recordtableGetRowCount() > 0) {
        const RecordTableData *recordTable = node->recordtableGetTableData();
        for (int i=0; i<static_cast<int>(recordTable->size()); i++)
            readRecordTags(recordTable, i);
    }

    // Рекурсивная обработка каждой подчиненной ветки
    for (int i=0; i<node->childCount(); i++) {
        readNodeTagsRecursively(node->child(i));
    }
}


void TagsModel::readRecordTags(const RecordTableData *searchRecordTable, int index) {
    Record *record = searchRecordTable->getRecord(index);
    QString tagsString = searchRecordTable->getField("tags", index);

    // Разделяем строку на отдельные метки
    QStringList tagsList = tagsString.split(QRegExp(TAGS_SEPARATORS_PATTERN), Qt::SkipEmptyParts);

    // Перебираем все метки
    foreach (const QString &tag, tagsList) {
        QString tagValue = tag.trimmed().toLower();

        if (tagValue.isEmpty())
            continue;

        if (data.contains(tagValue)) {
            // Если метка уже была добавлена, тогда данную заметку просто добавляем в список
            data[tagValue]->insertRecordByTag(record);
        } else {
            // Иначе добавляем метку с данной заметкой
            auto *tableData = new RecordTableData();
            tableData->setTagName(tag);
            tableData->insertRecordByTag(record);
            data[tagValue] = tableData;
        }
    }
}


void TagsModel::addRow(const QString &tagName, const int recordsCount) {
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


QString TagsModel::getTagNameByIndex(const QModelIndex &index) {
    int columnIndex = 0;
    if (index.column() == columnIndex) {
        return index.data(USER_ROLE_TAG_NAME).toString();
    } else {
        QStandardItem *itemInfo = item(index.row(), columnIndex);
        return itemInfo->data(USER_ROLE_TAG_NAME).toString();
    }
}


QModelIndex TagsModel::getIndexByTagName(const QString &tagName) {
    QString tagNameLower = tagName.trimmed().toLower();
    
    // Проходим по всем строкам модели
    for (int row = 0; row < rowCount(); ++row) {
        QModelIndex index = this->index(row, 0);
        QString currentTagName = getTagNameByIndex(index);
        
        if (currentTagName.toLower() == tagNameLower) {
            return index;
        }
    }
    
    return QModelIndex();
}


RecordTableData* TagsModel::getRecordTableByIndex(const QModelIndex &index, const TreeItem *startNode) {
    QString tagName = getTagNameByIndex(index);
    qDebug() << "Get records list by tag [" << tagName << "]";

    RecordTableData *tableData = data[tagName];
    if (tableData == nullptr) {
        qDebug() << "Warning: Couldn't get a tableData by tag [" << tagName << "]";
        return nullptr;
    }

    // Заполняем таблицу заново (актуализируем)
    tableData->empty();
    QList<Record*> recordsByTag = findRecordsByTagRecursively(tagName, startNode);
    foreach (Record *record, recordsByTag) {
        tableData->insertRecordByTag(record);
    }

    return tableData;
}

QList<Record*> TagsModel::findRecordsByTagRecursively(const QString &tagName, const TreeItem *node) {
    QList<Record*> list;

    // Если данная ветка - "Избранное", то пропускаем ее
    if (node->getField("id") == FixedParameters::favoritesItemId)
        return list;

    // Если ветка зашифрована, и пароль не был введен
    if (node->getField("crypt")=="1" &&
        globalParameters.getCryptKey().length()==0
    ) {
        isUnsearchCryptBranchPresent = true;
        return list;
    }

    // Если у ветки есть записи, перебираем их
    if (node->recordtableGetRowCount() > 0) {
        const RecordTableData *recordTable = node->recordtableGetTableData();
        for (int i=0; i<static_cast<int>(recordTable->size()); i++) {
            Record *record = recordTable->getRecord(i);
            QString tagsString = recordTable->getField("tags", i);

            // Разделяем строку на отдельные метки
            QStringList tagsList = tagsString.split(QRegExp(TAGS_SEPARATORS_PATTERN), Qt::SkipEmptyParts);

            // Перебираем все метки
            foreach (const QString &tag, tagsList) {
                // Если метка совпадает, добавляем данную заметку в список
                QString tagValue = tag.trimmed().toLower();
                if (tagName == tagValue) {
                    list.append(record);
                }
            }
        }
    }

    // Рекурсивная обработка каждой подчиненной ветки
    for (int i=0; i<node->childCount(); i++) {
        foreach (Record *record, findRecordsByTagRecursively(tagName, node->child(i))) {
            list.append(record);
        }
    }

    return list;
}


QModelIndex TagsModel::renameTag(const QModelIndex &sourceIndex, const QString &newName) {
    QString newNameLower = newName.toLower();
    QString oldName = getTagNameByIndex(sourceIndex);

    if (newNameLower == oldName)
        return sourceIndex;

    QModelIndex resultIndex;
    int resultRow = -1;

    if (data.contains(oldName)) {
        RecordTableData *tableData = data.take(oldName);

        // Обновляем имя метки в записях
        for (int i = 0; i < tableData->size(); ++i) {
            QString oldTagsString = tableData->getField("tags", i);
            // Разделяем строку на отдельные метки
            QStringList tagsList = oldTagsString.split(QRegExp(TAGS_SEPARATORS_PATTERN), Qt::SkipEmptyParts);
            for (QString &tag : tagsList) {
                // Заменяем все совпадающие метки, т.к. их может быть несколько одинаковых у записи
                if (tag.trimmed().toLower() == oldName) {
                    tag = newName;
                }
            }

            QString newTagsString = tagsList.join(", ");
            tableData->setField("tags", newTagsString, i);
        }

        // Обновляем список меток
        if (data.contains(newNameLower)) {
            RecordTableData *existingTableData = data[newNameLower];

            // Если новое имя совпадает с уже существующей меткой,
            // перемещаем записи в список записей существующей метки
            for (int i=0; i<tableData->size(); ++i) {
                existingTableData->insertRecordByTag(tableData->getRecord(i));
            }
            tableData->empty();
            tableData = nullptr;

            // Удаляем из модели "старую" метку
            removeRow(sourceIndex.row());

            resultIndex = getIndexByTagName(newNameLower);
            if (resultIndex.isValid()) {
                resultRow = resultIndex.row();

                // Обновляем кол-во записей во второй колонке существующей метки
                QStandardItem *countItem = item(resultRow, 1);
                if (countItem) {
                    unsigned int recordsCount = existingTableData->size();
                    QString recordsCountString;
                    countItem->setText(recordsCountString.setNum(recordsCount));
                    countItem->setData(recordsCount, SORT_ROLE);
                }
            }
        } else {
            // Если новое имя уникально, добавляем новый элемент в список
            data[newNameLower] = tableData;

            // Обновляем имя метки в первой колонке
            resultIndex = sourceIndex;
            resultRow = sourceIndex.row();
            QStandardItem *nameItem = item(resultRow, 0);
            if (nameItem) {
                nameItem->setText(newNameLower);
                nameItem->setData(newNameLower, USER_ROLE_TAG_NAME);
                nameItem->setData(newNameLower, SORT_ROLE);
            }
        }
    }

    if (resultRow > -1) {
        // Уведомляем представление об изменении данных
        QModelIndex topLeft = index(resultRow, 0);
        QModelIndex bottomRight = index(resultRow, 1);
        emit dataChanged(topLeft, bottomRight);
    }

    return resultIndex;
}


void TagsModel::deleteTag(const QModelIndex &sourceIndex) {
    QString tagName = getTagNameByIndex(sourceIndex);
    if (data.contains(tagName)) {
        RecordTableData *tableData = data.take(tagName);

        // Удаляем метку из записей
        for (int i = 0; i < tableData->size(); ++i) {
            QString oldTagsString = tableData->getField("tags", i);
            // Разделяем строку на отдельные метки
            QStringList tagsList = oldTagsString.split(QRegExp(TAGS_SEPARATORS_PATTERN), Qt::SkipEmptyParts);
            for (QString &tag : tagsList) {
                tag = tag.trimmed().toLower();
            }
            if (tagsList.removeOne(tagName)) {
                QString newTagsString = tagsList.join(", ");
                tableData->setField("tags", newTagsString, i);
            } else {
                qDebug() << "Tag delete error: not found tag [" << tagName << "] in record tags [" << oldTagsString << "]";
            }
        }
    }

    // Удаляем метку из модели
    int row = sourceIndex.row();
    if (row > -1) {
        removeRow(row);
    }
}
