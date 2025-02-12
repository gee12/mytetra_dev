#include <QWidget>
#include <QBoxLayout>
#include <QDir>
#include <QLineEdit>

#include "AppConfigPage_Tree.h"
#include "models/appConfig/AppConfig.h"
#include "libraries/GlobalParameters.h"


extern AppConfig mytetraConfig;
extern GlobalParameters globalParameters;


AppConfigPage_Tree::AppConfigPage_Tree(QWidget *parent) : ConfigPage(parent)
{
  setupUi();
  setupSignals();
  assembly();
}


AppConfigPage_Tree::~AppConfigPage_Tree()
{

}


void AppConfigPage_Tree::setupUi(void)
{
  qDebug() << "Create \"Tree\" config page";

  // Используется ли список избранных записей
  showFavorites=new QCheckBox(this);
  showFavorites->setText(tr("Show favorite records"));
  showFavorites->setChecked(mytetraConfig.get_showFavorites());

  // Блок настройки подтверждения для действия "cut" на ветке
  cutBranchConfirm=new QCheckBox(this);
  cutBranchConfirm->setText(tr("Confirm item cut"));
  cutBranchConfirm->setChecked(mytetraConfig.get_cutbranchconfirm());
}


void AppConfigPage_Tree::setupSignals(void)
{
}


void AppConfigPage_Tree::assembly(void)
{
  // Собирается основной слой
  QVBoxLayout *centralLayout=new QVBoxLayout();
  centralLayout->addWidget(showFavorites);
  centralLayout->addWidget(cutBranchConfirm);
  centralLayout->addStretch();

  // Основной слой устанавливается
  setLayout(centralLayout);
}


// Метод должен возвращать уровень сложности сделанных изменений
// 0 - изменения не требуют перезапуска программы
// 1 - изменения требуют перезапуска программы
int AppConfigPage_Tree::applyChanges(void)
{
  qDebug() << "Apply changes misc";

  int result=0;

  // Сохраняется настройка использования избранных записей
  if (mytetraConfig.get_showFavorites() != showFavorites->isChecked()) {
      mytetraConfig.set_showFavorites(showFavorites->isChecked());
      result=1;
  }

  // Сохраняется настройка подтверждения для действия "cut" на ветке
  if (mytetraConfig.get_cutbranchconfirm() != cutBranchConfirm->isChecked())
    mytetraConfig.set_cutbranchconfirm(cutBranchConfirm->isChecked());

  return result;
}
