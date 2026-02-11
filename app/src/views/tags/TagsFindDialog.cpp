#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QtGlobal>
#include <QtDebug>
#include <QShowEvent>

#include "TagsFindDialog.h"


TagsFindDialog::TagsFindDialog(QWidget *parent) : QDialog(parent) {
    setupUI();
    setupSignals();
    assembly();

    QShowEvent event;
    showEvent(&event);
}


void TagsFindDialog::setupUI() {
    lineEdit = new QLineEdit();
    lineEdit->setMinimumWidth(120);

    searchBackward = new QCheckBox(tr("Search &backward"));

    findButton = new QPushButton(tr("&Find"));
    findButton->setDefault(true);
    findButton->setEnabled(false);

    this->setWindowTitle(tr("Find in tags"));
}


void TagsFindDialog::setupSignals() {
    connect(lineEdit, &QLineEdit::textChanged, this, &TagsFindDialog::enableFindButton);
    connect(findButton, &QPushButton::clicked, this, &TagsFindDialog::findClicked);
}


void TagsFindDialog::assembly() {
    auto *findLineLayout = new QHBoxLayout();
    findLineLayout->addWidget(lineEdit);
    findLineLayout->addWidget(findButton);

    auto *centralLayout = new QVBoxLayout();
    centralLayout->addLayout(findLineLayout);
    centralLayout->addWidget(searchBackward);

    this->setLayout(centralLayout);

    this->setWindowFlags(Qt::Dialog
                         | Qt::WindowTitleHint
                         | Qt::MSWindowsFixedSizeDialogHint
                         | Qt::WindowCloseButtonHint);
}


void TagsFindDialog::findClicked() {
    QString text = lineEdit->text();

    QTextDocument::FindFlags flags = 0;
    if (searchBackward->isChecked()) flags |= QTextDocument::FindBackward;

    emit onFindTag(text, flags);
}


void TagsFindDialog::enableFindButton(const QString &text) {
    // Кнопка поиска активна только тогда, когда есть текст для поиска
    findButton->setEnabled(!text.isEmpty());
}
