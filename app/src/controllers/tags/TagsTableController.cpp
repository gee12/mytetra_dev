#include <QObject>

#include "main.h"
#include "TagsTableController.h"

#include <QHeaderView>

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


TagsTableController::TagsTableController(QObject *parent) : QObject(parent)
{
    view = new TagsTableWidget( qobject_cast<QWidget *>(parent) );
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


TagsTableController::~TagsTableController()
{
}


void TagsTableController::loadTags()
{
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


void TagsTableController::onTagClicked(const QModelIndex &proxyIndex) const {
    qDebug() << "onTagClicked";

    auto *treeScreen = find_object<TreeScreen>("treeScreen");
    // Убираем текущее выделение ветки в дереве
    treeScreen->clearSelection();

    QModelIndex sourceIndex = proxyModel->mapToSource(proxyIndex);
    RecordTableData *recordTableByTag = sourceModel->getRecordTableByIndex(sourceIndex, treeScreen->knowTreeModel->rootItem);

    auto *tagsScreen = find_object<TagsScreen>("tagsScreen");
    tagsScreen->onTagSelected(recordTableByTag);
}

void TagsTableController::onSortChanged(const int columnIndex, const Qt::SortOrder order) const {
    qDebug() << "onTagsHeaderClicked: index " << columnIndex << ", order " << order;

    const QString sort = QString("%1,%2").arg(columnIndex).arg(order);
    mytetraConfig.set_tags_sort(sort);
}

void TagsTableController::findInTags() {
    //TODO
}


TagsTableWidget *TagsTableController::getView()
{
    return view;
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


