#ifndef _TAGSSCREEN_H_
#define	_TAGSSCREEN_H_

#include <QWidget>

#include "TagsFindDialog.h"
#include "controllers/recordTable/RecordTableController.h"
#include "controllers/tags/TagsTableController.h"

class QLineEdit;
class QPushButton;
class QToolButton;
class QHBoxLayout;
class QVBoxLayout;
class QGridLayout;
class QLabel;
class QCheckBox;
class QProgressDialog;
class QToolBar;

class KnowTreeModel;
class TreeItem;
class TagsTableWidget;

// Виджет списка меток базы

class TagsScreen : public QWidget
{
 Q_OBJECT

public:
 TagsScreen(QWidget *parent=nullptr);
 virtual ~TagsScreen();

 void reloadTags();
 void clearTags();
 void showTag(const QString &tagName);
 void setWarningMessage(const QString &warningMessage);
 void findInTags();
 int restoreWidth();
 void saveWidth();

public slots:
 void widgetShow();
 void widgetHide();

private:
 TagsTableController *controller;

 QAction *actionFind;
 QAction *actionClose;
 QToolBar *toolBar;

 QHBoxLayout *headerLayout;
 QLabel *headerLabel;
 QHBoxLayout *headerLine;
 QGridLayout *headerGrid;

 QLabel *warningLabel;

 TagsTableWidget *tagsTableWidget;

 QVBoxLayout *mainLayout;

 TagsFindDialog *findDialog;

 void setupActions();
 void setupUI();
 void setupHeaderUI();
 void assembly();
 void assemblyHeaderLayout();
 void setupSignals();
};

#endif	/* _TAGSSCREEN_H_ */

