#include "EditorToolbarSettingsAvailableToolsModel.h"

#include "main.h"
#include "views/record/MetaEditor.h"
#include "libraries/wyedit/EditorToolBarAssistant.h"
#include "libraries/wyedit/EditorConfig.h"


EditorToolbarSettingsAvailableToolsModel::EditorToolbarSettingsAvailableToolsModel(QObject *parent) :
    EditorToolbarSettingsAbstractModel(parent)
{

}


void EditorToolbarSettingsAvailableToolsModel::init()
{
    // Все уже используемые инструменты на обоих линиях панели
    QStringList commandsInToolsLine = editorConfig->get_tools_line_1().split(',');
    commandsInToolsLine.append(editorConfig->get_tools_line_2().split(','));

    // Список названий всех контролов (команд) панелей инструментов
    EditorToolBarAssistant *editorToolBarAssistant = find_object<MetaEditor>("editorScreen")->editorToolBarAssistant;
    QStringList *commandNameList = editorToolBarAssistant->getCommandNameList();
    for (int i=0; i!=commandNameList->size(); ++i) {
        QString command = commandNameList->at(i);

        // Для десктопной версии пропускаем кнопки, нужные для мобильной версии
        // todo: подумать, а надо ли это действие
        if (editorToolBarAssistant->getViewMode() == Editor::WYEDIT_DESKTOP_MODE && (command=="back" || command=="find_in_base")) {
            continue;
        }

        // Добавление только тех команд, которые отсутствуют на обоих панелях инструментов
        if (commandsInToolsLine.indexOf(command)==-1) {
            QStandardItem *item=new QStandardItem(command);
            item->setIcon( editorToolBarAssistant->getIcon(command) );
            this->appendRow(item); // Элемент отдается во владение модели
        }
    }

    // Создание неудаляемого элемента <РАЗДЕЛИТЕЛЬ>
    this->insertRow(0, new QStandardItem(tr("<SEPARATOR>")));

    emit layoutChanged();
}