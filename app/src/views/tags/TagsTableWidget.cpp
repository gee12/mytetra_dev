#include <QLayout>
#include <QString>
#include <QHeaderView>
#include <QTableView>

#include "TagsTableWidget.h"
#include "main.h"
#include "views/mainWindow/MainWindow.h"
#include "models/appConfig/AppConfig.h"
#include "libraries/helpers/GestureHelper.h"
#include "libraries/helpers/CssHelper.h"

extern AppConfig mytetraConfig;


TagsTableWidget::TagsTableWidget(QWidget *parent) : QWidget(parent)
{
  setupUI();
  setupSignals();
  assembly();
}


TagsTableWidget::~TagsTableWidget()
{
}


void TagsTableWidget::setupUI()
{
  tagsTableView = new QTableView(this);
  tagsTableView->setObjectName("tagsTableView");
  tagsTableView->setMinimumSize(200,1);

  // Включение сортировки
  tagsTableView->setSortingEnabled(true);

  // Запрещается передвижение заголовков столбцов
  tagsTableView->horizontalHeader()->setSectionsMovable(false);

  // Растянуть столбцы таблицы о ширине
  tagsTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
  tagsTableView->horizontalHeader()->setMinimumSectionSize(100);

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

  // Видимость горизонтального скролла
  tagsTableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}


void TagsTableWidget::setupSignals()
{
  // Подключаем сигнал изменения размера столбцов
  connect(tagsTableView->horizontalHeader(), &QHeaderView::sectionResized,
          this, &TagsTableWidget::onSectionResized);
}


void TagsTableWidget::assembly()
{
  auto *centralLayout = new QHBoxLayout();

  centralLayout->addWidget(tagsTableView);
  centralLayout->setContentsMargins(0,0,0,0);

  centralLayout->setSpacing(0);

  this->setLayout(centralLayout);
}


void TagsTableWidget::clearAll()
{
  setOverdrawMessage("");
}


void TagsTableWidget::onDataLoaded()
{
    const auto *model = tagsTableView->model();
    if (model->rowCount()==0)
    {
        setOverdrawMessage(tr("Tags not found."));
    }
}


void TagsTableWidget::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);

  if (overdrawMessage.length() > 0)
  {
    QPainter painter(this);
    painter.setPen( QApplication::palette().color(QPalette::ToolTipText) );
    painter.drawText(rect(), Qt::AlignCenter, overdrawMessage);
  }
}


void TagsTableWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);

  // Общая ширина виджета без полосы прокрутки
  int widgetWidth = this->width() - 22;

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
  int widgetWidth = this->width() - 22;

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
  // чтобы общая ширина теблицы оставалась неизменной
  if (logicalIndex < columnCount - 1) {
    int currentSizeForNextColumn = tagsTableView->horizontalHeader()->sectionSize(logicalIndex + 1);
    int newSizeForNextColumn = qMax(minSize, currentSizeForNextColumn - excess);

    // Временно отключаем сигнал, чтобы избежать рекурсии
    tagsTableView->horizontalHeader()->blockSignals(true);
    tagsTableView->horizontalHeader()->resizeSection(logicalIndex + 1, newSizeForNextColumn);
    tagsTableView->horizontalHeader()->blockSignals(false);
  }
}


void TagsTableWidget::setOverdrawMessage(const QString message)
{
  // Установка надписи, которая появляется поверх виджета
  overdrawMessage = message;

  if (overdrawMessage.length() > 0)
    tagsTableView->hide(); // Скрывается виджет таблицы, потому что он перекрывает выводимую надпись
  else
    tagsTableView->show();

  // Обновляется внешний вид виджета
  update();
}

QTableView* TagsTableWidget::getTableView() {
    return tagsTableView;
}
