#include <QLayout>
#include <QString>
#include <QHeaderView>
#include <QTableView>
#include <QItemSelectionModel>
#include <QInputDialog>
#include <QScrollBar>
#include <QMessageBox>

#include "TagsTableWidget.h"
#include "main.h"
#include "controllers/tags/TagsTableController.h"
#include "views/mainWindow/MainWindow.h"
#include "models/appConfig/AppConfig.h"
#include "libraries/helpers/GestureHelper.h"
#include "libraries/helpers/CssHelper.h"

extern AppConfig mytetraConfig;


TagsTableWidget::TagsTableWidget(QWidget *parent, TagsTableController *controller) : QWidget(parent) {
  setController(controller);
  setupUI();
  controller->setView(this);
  setupActions();
  setupSignals();
  assembly();

  assemblyContextMenu();
}


TagsTableWidget::~TagsTableWidget() {
}


void TagsTableWidget::setController(TagsTableController *controller) {
  this->controller = controller;
}


void TagsTableWidget::setupUI() {
  tagsTableView = new QTableView(this);
  tagsTableView->setObjectName("tagsTableView");
  tagsTableView->setMinimumSize(100,1);

  // Включение сортировки
  tagsTableView->setSortingEnabled(true);

  // Запрещается передвижение заголовков столбцов
  tagsTableView->horizontalHeader()->setSectionsMovable(false);

  // Растянуть столбцы таблицы о ширине
  tagsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  tagsTableView->horizontalHeader()->setDefaultSectionSize(100);
  tagsTableView->horizontalHeader()->setMinimumSectionSize(50);

  // Установка высоты строки с принудительной стилизацией (если это необходимо),
  // так как стилизация через QSS для элементов QTableView полноценно не работает
  // У таблицы есть вертикальные заголовки, для каждой строки, в которых отображается номер строки.
  // При задании высоты вертикального заголовка, высота применяется и для всех ячеек в строке.
  tagsTableView->verticalHeader()->setDefaultSectionSize(tagsTableView->verticalHeader()->minimumSectionSize());
  int height = mytetraConfig.getUglyQssReplaceHeightForTableView();
  if (height!=0)
    tagsTableView->verticalHeader()->setDefaultSectionSize(height);
  if (mytetraConfig.getInterfaceMode()=="mobile")
    tagsTableView->verticalHeader()->setDefaultSectionSize(CssHelper::getCalculateIconSizePx());

  // Отключаем номера строк
  tagsTableView->verticalHeader()->setVisible(false);

  // Могут выделяться только строки, а не отдельный item таблицы
  tagsTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
  tagsTableView->setSelectionMode(QAbstractItemView::SingleSelection);

  // Редактирование невозможно
  tagsTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

  // Настройка области виджета для кинетической прокрутки
  GestureHelper::setKineticScrollArea( qobject_cast<QAbstractItemView*>(tagsTableView) );

  // Убираем выделение заголовков
  tagsTableView->horizontalHeader()->setHighlightSections(false);

  // Видимость скролла
  tagsTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  tagsTableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}


void TagsTableWidget::setupActions() {
  // Переименование метки
  actionRenameTag = new QAction(tr("Rename"), this);
  actionRenameTag->setIcon(QIcon(":/resource/pic/note_edit.svg"));

  // Копирование ссылки на метку
  actionCopyTagReference = new QAction(tr("Copy tag reference"), this);
  actionCopyTagReference->setIcon(QIcon(":/resource/pic/note_reference.svg"));

  // Удаление метки
  actionDeleteTag = new QAction(tr("Delete"), this);
  actionDeleteTag->setIcon(QIcon(":/resource/pic/note_delete.svg"));
}


void TagsTableWidget::setupSignals() {
  // Изменение выделения в таблице
  connect(tagsTableView->selectionModel(), &QItemSelectionModel::selectionChanged, controller, &TagsTableController::onSelectionChanged);
  // Сортировка меток
  connect(tagsTableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged, controller, &TagsTableController::onSortChanged);
  // Изменение размера столбцов
  connect(tagsTableView->horizontalHeader(), &QHeaderView::sectionResized,
          this, &TagsTableWidget::onSectionResized);
  // Контекстное меню по правому клику на списке меток
  connect(tagsTableView, &QTableView::customContextMenuRequested,
          this, &TagsTableWidget::onCustomContextMenuRequested);
  // Контекстное меню по долгому нажатию
  //connect(tagsTableView, &QTableView::tapAndHoldGestureFinished,
  //        this, &TagsTableWidget::onCustomContextMenuRequested);
  // Переименование метки
  connect(actionRenameTag, &QAction::triggered, this, &TagsTableWidget::onRenameTagContext);
  // Копирование ссылки на метку
  connect(actionCopyTagReference, &QAction::triggered, controller, &TagsTableController::onCopyTagReferenceContext);
  // Удаление метки
  connect(actionDeleteTag, &QAction::triggered, this, &TagsTableWidget::onDeleteTagContext);
}


void TagsTableWidget::assemblyContextMenu() {
  contextMenu = new QMenu(this);

  contextMenu->addAction(actionRenameTag);
  contextMenu->addAction(actionCopyTagReference);
  contextMenu->addAction(actionDeleteTag);

  tagsTableView->setContextMenuPolicy(Qt::CustomContextMenu);
}


void TagsTableWidget::assembly() {
  auto *centralLayout = new QHBoxLayout();

  centralLayout->addWidget(tagsTableView);
  centralLayout->setContentsMargins(0,0,0,0);

  centralLayout->setSpacing(0);

  this->setLayout(centralLayout);
}


void TagsTableWidget::clearAll() {
  setOverdrawMessage("");
}


void TagsTableWidget::onDataLoaded() {
    const auto *model = tagsTableView->model();
    if (model->rowCount()==0) {
        setOverdrawMessage(tr("Tags not found."));
    }
}


void TagsTableWidget::paintEvent(QPaintEvent *event) {
  QWidget::paintEvent(event);

  if (overdrawMessage.length() > 0) {
    QPainter painter(this);
    painter.setPen( QApplication::palette().color(QPalette::ToolTipText) );
    painter.drawText(rect(), Qt::AlignCenter, overdrawMessage);
  }
}


void TagsTableWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);

  // Общая ширина виджета без полосы прокрутки
  int widgetWidth = this->width() - getVerticalScrollBarWidth();

  // Если виджет еще не показан, выходим
  if (widgetWidth <= 0) {
    return;
  }

  // Вычисляем текущую общую ширину всех столбцов
  int totalColumnsWidth = 0;
  int columnCount = tagsTableView->horizontalHeader()->count();

  for (int i = 0; i < columnCount; ++i) {
    totalColumnsWidth += tagsTableView->horizontalHeader()->sectionSize(i);
  }

  if (totalColumnsWidth > 0) {
    // Вычисляем коэффициент масштабирования
    double scaleFactor = static_cast<double>(widgetWidth) / static_cast<double>(totalColumnsWidth);

    // Применяем масштабирование ко всем столбцам
    for (int i = 0; i < columnCount; ++i) {
      int newColumnSize = static_cast<int>(tagsTableView->horizontalHeader()->sectionSize(i) * scaleFactor);
      // Учитываем минимальный размер столбца
      int minSize = tagsTableView->horizontalHeader()->minimumSectionSize();
      newColumnSize = qMax(minSize, newColumnSize);

      tagsTableView->horizontalHeader()->blockSignals(true);
      tagsTableView->horizontalHeader()->resizeSection(i, newColumnSize);
      tagsTableView->horizontalHeader()->blockSignals(false);
    }
  }
}

void TagsTableWidget::onSectionResized(int logicalIndex, int oldSize, int newSize) {
  Q_UNUSED(oldSize)
  Q_UNUSED(newSize)

  // Общая ширина виджета без полосы прокрутки
  int widgetWidth = this->width() - getVerticalScrollBarWidth();

  // Если виджет еще не показан, выходим
  if (widgetWidth <= 0) {
    return;
  }

  // Вычисляем текущую общую ширину всех столбцов
  int totalColumnsWidth = 0;
  int columnCount = tagsTableView->horizontalHeader()->count();
  for (int i = 0; i < columnCount; ++i) {
    totalColumnsWidth += tagsTableView->horizontalHeader()->sectionSize(i);
  }

  int excess = totalColumnsWidth - widgetWidth;
  int minSize = tagsTableView->horizontalHeader()->minimumSectionSize();

  // Если общая ширина по столбцам превышает максимальную, ограничиваем измененный столбец
  if (totalColumnsWidth > widgetWidth) {
    int currentSize = tagsTableView->horizontalHeader()->sectionSize(logicalIndex);
    int newSize = qMax(minSize, currentSize - excess);

    // Временно отключаем сигнал, чтобы избежать рекурсии
    tagsTableView->horizontalHeader()->blockSignals(true);
    tagsTableView->horizontalHeader()->resizeSection(logicalIndex, newSize);
    tagsTableView->horizontalHeader()->blockSignals(false);
  }

  // Меняем ширину следующего столбца (если он есть),
  // чтобы общая ширина таблицы оставалась неизменной
  if (logicalIndex < columnCount - 1) {
    int currentSizeForNextColumn = tagsTableView->horizontalHeader()->sectionSize(logicalIndex + 1);
    int newSizeForNextColumn = qMax(minSize, currentSizeForNextColumn - excess);

    // Временно отключаем сигнал, чтобы избежать рекурсии
    tagsTableView->horizontalHeader()->blockSignals(true);
    tagsTableView->horizontalHeader()->resizeSection(logicalIndex + 1, newSizeForNextColumn);
    tagsTableView->horizontalHeader()->blockSignals(false);
  }
}


void TagsTableWidget::onCustomContextMenuRequested(const QPoint &mousePos) {
  // Отображение контекстного меню
  contextMenu->exec(tagsTableView->viewport()->mapToGlobal(mousePos));
}


void TagsTableWidget::onRenameTagContext() {
  QModelIndex proxyIndex = getFirstSelectedIndex();
  if (proxyIndex.isValid()) {
    // Получение имени ветки
    QString oldName = proxyIndex.data(USER_ROLE_TAG_NAME).toString();

    // Создается окно ввода данных
    onRenameTagRecursively(proxyIndex, oldName);
  }
}

void TagsTableWidget::onRenameTagRecursively(QModelIndex proxyIndex, QString oldName) {
    // Создается окно ввода данных
    bool result;
    QString newName = QInputDialog::getText(this,
                                            tr("Rename tag"),
                                            tr("Tag name:"),
                                            QLineEdit::Normal,
                                            oldName,
                                            &result);
    if (result && !newName.trimmed().isEmpty()) {
      if (newName.contains(QRegExp(TAGS_SEPARATORS_PATTERN))) {
        // Уведомляем, что имя конкретной метки не должно содержать знаки разделения меток.
        //TODO: Возможно, следует добавить к записям все введенные метки.
        QMessageBox messageBox(tagsTableView);
        messageBox.setWindowTitle(tr("Invalid characters"));
        messageBox.setText(tr("The tag name must not contain the tags separation characters ',' or ';'."));
        messageBox.addButton(tr("OK"),QMessageBox::AcceptRole);
        messageBox.exec();

        onRenameTagRecursively(proxyIndex, newName);
      } else {
        controller->renameTag(proxyIndex, newName);
      }
    }
}

void TagsTableWidget::onDeleteTagContext() {
  QModelIndex proxyIndex = getFirstSelectedIndex();
  if (proxyIndex.isValid()) {
    QString tagName = proxyIndex.data(USER_ROLE_TAG_NAME).toString();

    QMessageBox messageBox(tagsTableView);
    messageBox.setWindowTitle(tr("Delete tag"));
    messageBox.setText(tr("Are you sure to delete tag \"%1\" from records?").arg(tagName));
    messageBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
    QAbstractButton *deleteButton = messageBox.addButton(tr("Delete"), QMessageBox::AcceptRole);

    messageBox.exec();
    if (messageBox.clickedButton() == deleteButton)
    {
      controller->deleteTag(proxyIndex);
    }
  }
}


void TagsTableWidget::setOverdrawMessage(const QString message) {
  // Установка надписи, которая появляется поверх виджета
  overdrawMessage = message;

  if (overdrawMessage.length() > 0)
    tagsTableView->hide(); // Скрывается виджет таблицы, потому что он перекрывает выводимую надпись
  else
    tagsTableView->show();

  // Обновляется внешний вид виджета
  update();
}


void TagsTableWidget::selectTableRow(const QModelIndex &proxyIndex) {
  int pos = proxyIndex.row();
  tagsTableView->selectRow(pos);
  tagsTableView->setFocus();
  tagsTableView->scrollTo(proxyIndex);
}


QModelIndex TagsTableWidget::getFirstSelectedIndex() {
  QModelIndexList selectItems = tagsTableView->selectionModel()->selectedIndexes();

  if (!selectItems.isEmpty()) {
    return selectItems.at(0);
  }
  return QModelIndex();
}


void TagsTableWidget::setSectionsSizes(QList<int> sizes) {
  if (sizes.size() >= 2) {
    auto horizontalHeader = tagsTableView->horizontalHeader();

    horizontalHeader->blockSignals(true);
    horizontalHeader->resizeSection(0, sizes[0]);
    horizontalHeader->resizeSection(1, sizes[1]);
    horizontalHeader->blockSignals(false);
  }
}


QList<int> TagsTableWidget::getSectionsSizes() {
  auto horizontalHeader = tagsTableView->horizontalHeader();
  return {
    horizontalHeader->sectionSize(0),
    horizontalHeader->sectionSize(1)
  };
}


QTableView* TagsTableWidget::getTableView() {
    return tagsTableView;
}


int TagsTableWidget::getVerticalScrollBarWidth() {
  return tagsTableView->verticalScrollBar()->width();
}