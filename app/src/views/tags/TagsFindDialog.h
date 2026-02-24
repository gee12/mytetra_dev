#ifndef _TAGSFINDDIALOG_H_
#define	_TAGSFINDDIALOG_H_

#include <QWidget>
#include <QDialog>
#include <QTextDocument>


class QCheckBox;
class QLineEdit;
class QPushButton;


class TagsFindDialog : public QDialog
{
 Q_OBJECT

public:
 TagsFindDialog(QWidget *parent=nullptr);
 
signals:
 void onFindTag(const QString &text, QTextDocument::FindFlags flags);

private slots:
 void findClicked();
 void enableFindButton(const QString &text);
 
private:
 QLineEdit *lineEdit;
 QCheckBox *searchBackward;
 QPushButton *findButton;
 
 void setupUI();
 void setupSignals();
 void assembly();
};

#endif	/* _TAGSFINDDIALOG_H_ */

