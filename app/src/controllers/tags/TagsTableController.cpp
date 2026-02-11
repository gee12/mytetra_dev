#include <QObject>

#include "main.h"
#include "TagsTableController.h"

#include <QHeaderView>

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
#include "views/tags/TagsTableWidget.h"
#include "views/tree/TreeScreen.h"


extern GlobalParameters globalParameters;
extern AppConfig mytetraConfig;
extern WalkHistory walkHistory;


TagsTableController::TagsTableController(QObject *parent) : QObject(parent) {
    view = new TagsTableWidget(qobject_cast<QWidget *>(parent), this);
    view->setObjectName("tagsTableView");

    // Создание модели данных
    sourceModel = new TagsModel(this);
    sourceModel->setObjectName("sourceModel");
    sourceModel->setHorizontalHeaderLabels({ tr("Tag name"), tr("Count") });

    // Создание модели для сортировки и фильтрации данных
    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(sourceModel);
    proxyModel->setObjectName("proxyModel");
    proxyModel->setSortRole(SORT_ROLE);

    // Модель данных задается для вида
    view->getTableView()->setModel(proxyModel);

    sortTags();
}


TagsTableController::~TagsTableController() {
}


void TagsTableController::loadTags() {
    clearData();

    auto *knowTreeModel = find_object<TreeScreen>("treeScreen")->knowTreeModel;
    TreeItem *startNode = knowTreeModel->rootItem;
    sourceModel->init(startNode);

    view->onDataLoaded();

    if (sourceModel->isUnsearchCryptBranchPresent) {
        auto *tagsScreen = find_object<TagsScreen>("tagsScreen");
        tagsScreen->setWarningMessage(tr("Storage has not been decrypted, and not all tags will be displayed."));
    }
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

    RecordTableData *recordTableByTag = sourceModel->getRecordTableByIndex(sourceIndex, treeScreen->knowTreeModel->rootItem);
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
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    selectTag(sourceIndex);
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


TagsTableWidget *TagsTableController::getView()
{
    return view;
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


void TagsTableController::sortTags() {
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

    qDebug() << "Rename tag [" << oldName << "] to [" << newName << "]";

    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);

    auto mainWindow = find_object<MainWindow>("mainwindow");
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

        RecordTableData *recordTableByTag = sourceModel->getRecordTableByIndex(targetSourceIndex, treeScreen->knowTreeModel->rootItem);
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
        // Принудительно "кликаем" метку, которая автоматически стала выделенной после удаления предыдущей
        onTagClicked(newProxyIndex);
    } else {
        // Устанавливаем пустые данные для отображения таблицы конечных записей
        find_object<RecordTableController>("recordTableController")->setTableData(nullptr);
    }
}