#include <QObject>

#include "main.h"
#include "TagsTableController.h"

#include <QHeaderView>
#include <QItemSelectionModel>

#include "libraries/FixedParameters.h"
#include "views/recordTable/RecordTableView.h"
#include "views/recordTable/RecordTableScreen.h"
#include "views/tags/TagsScreen.h"
#include "views/mainWindow/MainWindow.h"
#include "models/appConfig/AppConfig.h"
#include "libraries/GlobalParameters.h"
#include "libraries/WalkHistory.h"
#include "libraries/helpers/ObjectHelper.h"
#include "models/tree/KnowTreeModel.h"
#include "models/tags/TagsModel.h"
#include "views/tags/TagsTableWidget.h"
#include "views/tree/TreeScreen.h"


extern GlobalParameters globalParameters;
extern AppConfig mytetraConfig;
extern WalkHistory walkHistory;


TagsTableController::TagsTableController(QObject *parent) : QObject(parent) {
    initModels();
}


TagsTableController::~TagsTableController() {
}


void TagsTableController::initModels() {
    // Создание модели данных
    sourceModel = new TagsModel(this);
    sourceModel->setObjectName("sourceModel");
    sourceModel->setHorizontalHeaderLabels({ tr("Tag name"), tr("Count") });

    // Создание модели для сортировки и фильтрации данных
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(sourceModel);
    proxyModel->setObjectName("proxyModel");
    proxyModel->setSortRole(SORT_ROLE);
}


void TagsTableController::setView(TagsTableWidget *view) {
    this->view = view;

    // Для вида задается модель данных
    view->getTableView()->setModel(proxyModel);

    // Включается сортировка по нужному столбцу
    setSortIndicator();
}


void TagsTableController::loadData() {
    // Сохраняем номер строки и имя выделенной метки
    int selectedRow = -1;
    QString selectedTagName;
    const QModelIndex selectedIndex = view->getFirstSelectedIndex();
    if (selectedIndex.isValid()) {
        selectedRow = selectedIndex.row();
        selectedTagName = selectedIndex.data(USER_ROLE_TAG_NAME).toString();
    }

    clearData();

    auto *knowTreeModel = find_object<TreeScreen>("treeScreen")->knowTreeModel;
    const TreeItem *startNode = knowTreeModel->getRootItem();
    sourceModel->init(startNode);

    // Восстанавливаем выделение метки
    const QModelIndex proxyIndex = getProxyIndex(selectedRow, selectedTagName);
    view->onDataLoaded(proxyIndex);

    if (sourceModel->isUnsearchCryptBranchPresent) {
        auto *tagsScreen = find_object<TagsScreen>("tagsScreen");
        tagsScreen->setWarningMessage(tr("Storage has not been decrypted, and not all tags will be displayed."));
    }
}


QModelIndex TagsTableController::getProxyIndex(const int row, const QString &tagName) const {
    if (tagName.isEmpty())
        return QModelIndex();

    const int rowCount = proxyModel->rowCount();

    if (row >= 0 && row < rowCount) {
        const QModelIndex atSavedRow = proxyModel->index(row, 0);
        if (atSavedRow.data(USER_ROLE_TAG_NAME).toString() == tagName) {
            return atSavedRow;
        }
    }

    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex idx = proxyModel->index(row, 0);
        if (idx.data(USER_ROLE_TAG_NAME).toString() == tagName) {
            return idx;
        }
    }

    return QModelIndex();
}


void TagsTableController::clearData() {
    // Модель таблицы очищается
    sourceModel->setRowCount(0);
    sourceModel->setColumnCount(2);

    auto *tagsScreen = find_object<TagsScreen>("tagsScreen");
    tagsScreen->setWarningMessage(tr(""));

    view->clearAll();
}


void TagsTableController::clearSelection() const {
    view->getTableView()->clearSelection();
}


void TagsTableController::selectTag(const QModelIndex &sourceIndex) const {
    qDebug() << "TagsTableController::selectTag";

    // Убираем текущее выделение ветки в дереве
    auto *treeScreen = find_object<TreeScreen>("treeScreen");
    treeScreen->clearSelection();

    RecordTableData *recordTableByTag = sourceModel->getRecordTableByIndex(sourceIndex, treeScreen->knowTreeModel->getRootItem());
    find_object<RecordTableController>("recordTableController")->setTableData(recordTableByTag);
}


void TagsTableController::selectTag(const QString &tagName) const {
    QModelIndex sourceIndex = sourceModel->getIndexByTagName(tagName);

    // Выделяем метку в списке
    QModelIndex proxyIndex = proxyModel->mapFromSource(sourceIndex);
    view->selectTableRow(proxyIndex);

    // Устанавливаем список записей по метке
    selectTag(sourceIndex);
}


void TagsTableController::onTagClicked(const QModelIndex &proxyIndex) const {
    qDebug() << "TagsTableController::onTagClicked";

    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    selectTag(sourceIndex);
}


void TagsTableController::onSelectionChanged(const QItemSelection &selectedIndex, const QItemSelection &deselectedIndex) {
    Q_UNUSED(deselectedIndex)
    qDebug() << "TagsTableController::onSelectionChanged";

    if (!selectedIndex.indexes().isEmpty()) {
        auto proxyIndex = selectedIndex.indexes().at(0);
        onTagClicked(proxyIndex);
    }
}


void TagsTableController::onSortChanged(const int columnIndex, const Qt::SortOrder order) const {
    qDebug() << "onTagsHeaderClicked: index " << columnIndex << ", order " << order;

    const QString sort = QString("%1,%2").arg(columnIndex).arg(order);
    mytetraConfig.set_tags_sort(sort);
}


bool TagsTableController::checkRowMatching(const QString &text, int row) {
    QModelIndex index = proxyModel->index(row, 0);
    QString currentTagName = index.data(USER_ROLE_TAG_NAME).toString();

    if (currentTagName.contains(text, Qt::CaseInsensitive)) {
        qDebug() << "Found matching at row=" << row;
        view->selectTableRow(index);
        selectTag(index);
        return true;
    }
    return false;
}


void TagsTableController::findNextTag(const QString &text, QTextDocument::FindFlags flags) {
    qDebug() << "findNextTag: text=[" << text << "], flags=" << flags;

    int totalRows = proxyModel->rowCount();
    if (totalRows == 0) {
        qDebug() << "No rows in model";
        return;
    }

    // Определяем направление поиска
    bool isFindForward = !(flags & QTextDocument::FindBackward);

    qDebug() << "totalRows=" << totalRows << ", isFindForward=" << isFindForward;

    QModelIndex proxyIndex = view->getFirstSelectedIndex();
    if (proxyIndex.isValid()) {
        int currentRow = proxyIndex.row();
        qDebug() << "currentRow=" << currentRow;

        if (isFindForward) {
            // Поиск вперед: от следующей строки до конца, затем с начала до текущей
            int startRow = (currentRow == totalRows - 1) ? 0 : currentRow + 1;
            int endRow = totalRows;

            // Проходим от startRow до конца
            for (int row = startRow; row < endRow; ++row)
                if (checkRowMatching(text, row)) return;

            // Если не нашли и начинали не с начала, продолжаем с начала до текущей строки
            if (startRow > 0) {
                for (int row = 0; row <= currentRow; ++row)
                    if (checkRowMatching(text, row)) return;
            }
        } else {
            // Поиск назад: от предыдущей строки до начала, затем с конца до текущей
            int startRow = (currentRow == 0) ? totalRows - 1 : currentRow - 1;
            int endRow = -1; // -1 означает "до начала включительно"

            // Проходим от startRow до начала (включительно)
            for (int row = startRow; row > endRow; --row)
                if (checkRowMatching(text, row)) return;

            // Если не нашли и начинали не с конца, продолжаем с конца до текущей строки
            if (startRow < totalRows - 1) {
                for (int row = totalRows - 1; row >= currentRow; --row)
                    if (checkRowMatching(text, row)) return;
            }
        }

        qDebug() << "No match found";
    } else {
        // Если нет выделенной строки, начинаем поиск с начала (вперед) или с конца (назад)
        int startRow = isFindForward ? 0 : totalRows - 1;
        int endRow = isFindForward ? totalRows : -1;
        int step = isFindForward ? 1 : -1;

        for (int row = startRow; row != endRow; row += step)
            if (checkRowMatching(text, row)) return;

        qDebug() << "No match found";
    }
}


void TagsTableController::onCopyTagReferenceContext() {
    QModelIndex proxyIndex = view->getFirstSelectedIndex();
    if (proxyIndex.isValid()) {
        QString selectedTagName = proxyIndex.data(USER_ROLE_TAG_NAME).toString();
        QString reference = FixedParameters::appTextId + "://tag/" + selectedTagName;

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(reference);
    }
}


void TagsTableController::setSortIndicator() {
    // Включается сортировка по нужному столбцу
    QString sortString = mytetraConfig.get_tags_sort();
    if (!sortString.isEmpty() && sortString.contains(",")) {
        QStringList sortValues = sortString.split(",");
        if (sortValues.size() >= 2) {
            int index = sortValues[0].toInt();
            auto order = (Qt::SortOrder)sortValues[1].toInt();
            qDebug() << "Tags sort column index " << index << ", order " << order;

            proxyModel->sort(index, order);
            view->getTableView()->horizontalHeader()->setSortIndicator(index, order);
        }
    }
}


void TagsTableController::renameTag(const QModelIndex &proxyIndex, QString newName) {
    QString oldName = proxyIndex.data(USER_ROLE_TAG_NAME).toString();
    newName = newName.trimmed();

    if (oldName.trimmed() == newName)
        return;

    qDebug() << "Rename tag [" << oldName << "] to [" << newName << "]";

    // Для упрощения, берется только первая метка, если в новом имени метки фигурируют знаки разделения меток
    QStringList newTags = newName.split(QRegExp(TAGS_SEPARATORS_PATTERN), Qt::SkipEmptyParts);
    if (newTags.size() > 1) {
        newName = newTags.at(0).trimmed();
    }

    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);    auto mainWindow = find_object<MainWindow>("mainwindow");
    auto *treeScreen = find_object<TreeScreen>("treeScreen");

    mainWindow->setDisabled(true);

    // Переименовуем метку в модели
    QModelIndex targetSourceIndex = sourceModel->renameTag(sourceIndex, newName);
    // Сохраняем изменения в дереве
    treeScreen->saveKnowTree();

    mainWindow->setDisabled(false);

    if (sourceIndex == targetSourceIndex) {
        // Если после переименования метка не была объединена с другой, то выделение в списке меток не меняем.
        // Перевыбираем запись в списке, чтобы обновить поля в редакторе.
        auto *recordTableController = find_object<RecordTableController>("recordTableController");
        int recordPos = recordTableController->getFirstSelectionPos();
        if (recordPos > -1) {
            QModelIndex index = recordTableController->convertPosToProxyIndex(recordPos);
            recordTableController->clickToRecord(index);
        }
    } else {
        // Если после переименования метка не была объединена с другой, то выделяем результирующую метку.
        QModelIndex targetProxyIndex = proxyModel->mapFromSource(targetSourceIndex);
        view->selectTableRow(targetProxyIndex);

        RecordTableData *recordTableByTag = sourceModel->getRecordTableByIndex(targetSourceIndex, treeScreen->knowTreeModel->getRootItem());
        find_object<RecordTableController>("recordTableController")->setTableData(recordTableByTag);
    }

    // Если сортировка активна, proxyModel автоматически пересортирует строку
    // Но можно принудительно обновить сортировку:
    // int sortColumn = view->getTableView()->horizontalHeader()->sortIndicatorSection();
    // Qt::SortOrder sortOrder = view->getTableView()->horizontalHeader()->sortIndicatorOrder();
    // if (sortColumn >= 0) {
    //     proxyModel->sort(sortColumn, sortOrder);
    // }
}


void TagsTableController::deleteTag(QModelIndex proxyIndex) {
    QString tagName = proxyIndex.data(USER_ROLE_TAG_NAME).toString();
    qDebug() << "Delete tag [" << tagName << "]";

    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);

    auto mainWindow = find_object<MainWindow>("mainwindow");
    auto *treeScreen = find_object<TreeScreen>("treeScreen");

    mainWindow->setDisabled(true);

    // Удаляем метку из модели
    sourceModel->deleteTag(sourceIndex);
    // Сохраняем изменения в дереве
    treeScreen->saveKnowTree();

    mainWindow->setDisabled(false);

    QModelIndex newProxyIndex = view->getFirstSelectedIndex();
    if (newProxyIndex.isValid()) {
        // Принудительно выделяем метку, которая автоматически стала текущей после удаления предыдущей
        view->selectTableRow(newProxyIndex);
    } else {
        // Устанавливаем пустые данные для отображения таблицы конечных записей
        find_object<RecordTableController>("recordTableController")->setTableData(nullptr);
    }
}
