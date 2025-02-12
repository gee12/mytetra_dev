#ifndef _CONFIGPAGE_TREE_H_
#define	_CONFIGPAGE_TREE_H_

#include <QWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QPushButton>

#include "ConfigPage.h"


class AppConfigPage_Tree : public ConfigPage
{
 Q_OBJECT

public:
  AppConfigPage_Tree(QWidget *parent = nullptr);
  virtual ~AppConfigPage_Tree(void);

  int applyChanges(void);

protected:

  void setupUi(void);
  void setupSignals(void);
  void assembly(void);

  QCheckBox *showFavorites;           // Используется ли список избранных записей
  QCheckBox *cutBranchConfirm;        // Требуется ли показывать предупреждение при вырезании ветки
};


#endif	// _CONFIGPAGE_TREE_H_

