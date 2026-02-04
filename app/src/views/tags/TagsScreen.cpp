#include <QLineEdit>
#include <QGridLayout>
#include <QWidget>
#include <QBoxLayout>
#include <QLineEdit>
#include <QtGlobal>

#include "main.h"
#include "views/mainWindow/MainWindow.h"
#include "TagsScreen.h"
#include "TagsTableWidget.h"
#include "models/appConfig/AppConfig.h"
#include "views/record/MetaEditor.h"
#include "libraries/GlobalParameters.h"
#include "libraries/MtComboBox.h"
#include "libraries/helpers/ObjectHelper.h"
#include "controllers/tags/TagsTableController.h"
#include "libraries/helpers/ActionHelper.h"

extern AppConfig mytetraConfig;
extern GlobalParameters globalParameters;


TagsScreen::TagsScreen(QWidget *parent) : QWidget(parent)
{
  controller = new TagsTableController(this);
  controller->setObjectName("tagsTableController");

  setupActions();
  setupUI();
  assembly();
  setupSignals();
}


TagsScreen::~TagsScreen()
{
}


void TagsScreen::setupActions() {
  // Поиск по меткам
  actionFind = new QAction(this);
  //TODO
  actionFind->setIcon(QIcon(":/resource/pic/find_in_base.svg"));

  // Закрытие окна
  actionClose = new QAction(this);
  actionClose->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarCloseButton));
}


void TagsScreen::setupUI()
{
  setupHeaderUI();

  warningLabel = new QLabel();
  warningLabel->setWordWrap(true);
  warningLabel->setAlignment(Qt::AlignCenter);
  warningLabel->hide();

  // TagsTableWidget создается в контроллере
}


void TagsScreen::setupHeaderUI()
{
  headerLabel = new QLabel(tr("Tags"));

  toolBar = new QToolBar(this);
  insertActionAsButton(toolBar, actionFind);
  insertActionAsButton(toolBar, actionClose);
}


void TagsScreen::assembly()
{
  assemblyHeaderLayout();

  mainLayout = new QVBoxLayout();

  // header
  if (mytetraConfig.getInterfaceMode()=="desktop")
  {
    headerLine=new QHBoxLayout();
    headerLine->addLayout(headerLayout);

    mainLayout->addLayout(headerLine);
  }
  else if (mytetraConfig.getInterfaceMode()=="mobile")
  {
    headerGrid=new QGridLayout();
    headerGrid->addLayout(headerLayout, 0, 1);
    mainLayout->addLayout(headerGrid);
  }

  // warning
  mainLayout->addWidget(warningLabel);

  // table
  mainLayout->addWidget(controller->getView(), 10);

  // main
  mainLayout->setContentsMargins(0,0,0,0);
  mainLayout->setSizeConstraint(QLayout::SetNoConstraint);

  mainLayout->setContentsMargins(0,2,0,0);
  setLayout(mainLayout);
}


void TagsScreen::assemblyHeaderLayout()
{
  headerLayout = new QHBoxLayout();
  headerLayout->setContentsMargins(0,0,0,0);

  if (mytetraConfig.getInterfaceMode()=="desktop")
      headerLayout->addWidget(headerLabel);
  else if (mytetraConfig.getInterfaceMode()=="mobile")
      headerLabel->hide();

  headerLayout->addStretch();

  headerLayout->addWidget(toolBar);
}


void TagsScreen::setupSignals()
{
  // Поиск по меткам
  connect(actionFind, &QAction::triggered, controller, &TagsTableController::findInTags);
  // Закрытие окна
  connect(actionClose, &QAction::triggered, this, &TagsScreen::widgetHide);
}


void TagsScreen::reloadTags()
{
    qDebug() << "Start load tags from storage";
    // Непосредственный сбор меток по всем (расшифрованным) записям хранилища
    controller->loadTags();
}

void TagsScreen::showTag(const QString tagName) {
  // Показываем виджет, если не показан
  if (isVisible() == false)
    widgetShow();

  controller->selectTag(tagName);
}


void TagsScreen::setWarningMessage(const QString &warningMessage) {
  warningLabel->setText(warningMessage);
  warningLabel->setVisible(!warningMessage.isEmpty());
}


void TagsScreen::widgetShow()
{
  mytetraConfig.set_tagsscreen_show(true);
  this->show();
}


// Полное сокрытие виджета
void TagsScreen::widgetHide()
{
  // Сохранение размера сплиттера перед скрытием виджета
  QSplitter *hSplitter = find_object<QSplitter>("hSplitter");
  int size = hSplitter->sizes().at(2);
  mytetraConfig.set_tagsscreen_width(size);

  mytetraConfig.set_tagsscreen_show(false);
  this->close();
}
