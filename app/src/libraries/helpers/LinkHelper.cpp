#include <QUrl>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QDesktopServices>
#include <QDebug>

#include "LinkHelper.h"

#include "views/mainWindow/MainWindow.h"
#include "views/tags/TagsScreen.h"
#include "views/tree/KnowTreeView.h"
#include "models/tree/KnowTreeModel.h"
#include "libraries/FixedParameters.h"
#include "libraries/helpers/ObjectHelper.h"


LinkHelper::LinkHelper()
{

}


void LinkHelper::gotoReference(QString href)
{
    if(href.length()==0)
        return;

    // Попытка получения ID записи из ссылки
    QString recordId = getRecordIdFromInternalHref(href);
    if (!recordId.isEmpty()) {
        // Клик по внутренней ссылке на запись
        find_object<MainWindow>("mainwindow")->openRecordByInternalHref(recordId);
        return;
    }

    // Попытка получения имени метки из ссылки
    QString tagName = getTagNameFromInternalHref(href);
    if (!tagName.isEmpty()) {
        // Клик по внутренней ссылке на метку
        find_object<TagsScreen>("tagsScreen")->showTag(tagName);
        return;
    }

    openLinkWithDesktopServices( href );
}


bool LinkHelper::openLinkWithDesktopServices(const QString &link)
{
    // qDebug() << "Try open link " << link;

    QUrl url = QUrl(link);

    // Использовать метод QUrl::isLocalFile() нельзя, так как он просто
    // возвращает true если схема "file" и все.
    // Вместо этого написана специальная функция определения, внешняя это
    // или внутренняя ссылка
    if ( isExternal( url ) )
    {
        // Для внешних ссылок используется QDesktopServices
        return QDesktopServices::openUrl(url);
    }
    else
    {
        // Определено, что передана локальная ссылка

        QString filePath = link; // url.toLocalFile();

        // qDebug() << "Try open file " << filePath;

        // Используем QProcess для вызова системной команды
#ifdef Q_OS_WIN
        QStringList args;
        args << "/C" << "start" << "" << filePath.replace("/", "\\");
        QProcess::startDetached("cmd", args);
#elif defined(Q_OS_MAC)
        QProcess::startDetached("open", QStringList() << filePath);
#elif defined(Q_OS_LINUX)
        QProcess::startDetached("xdg-open", QStringList() << filePath);
#else
        return false; // Не поддерживается на других платформах
#endif
        return true;
    }
}


bool LinkHelper::isExternal(const QUrl &url)
{
    // Проверка схемы URL
    QString scheme = url.scheme().toLower();

    QStringList external = QStringList() << "http"
                                         << "https"
                                         << "ftp"
                                         << "sftp";

    return external.contains( scheme );
}


bool LinkHelper::isHrefInternal(QString href)
{
    if(href.contains(QRegExp("^"+FixedParameters::appTextId+":\\/\\/note\\/\\w+$")))
        return true;
    else
        return false;
}


bool LinkHelper::isHrefInternalToTag(QString href)
{
    return href.contains(QRegExp("^"+FixedParameters::appTextId+":\\/\\/tag\\/.+$"));
}


QString LinkHelper::getRecordIdFromInternalHref(QString href)
{
    if(!isHrefInternal(href))
        return "";

    href.replace(QRegExp("^"+FixedParameters::appTextId+":\\/\\/note\\/"), "");

    return href;
}


QString LinkHelper::getTagNameFromInternalHref(QString href)
{
    if (!isHrefInternalToTag(href))
        return "";

    href.replace(QRegExp("^"+FixedParameters::appTextId+":\\/\\/tag\\/"), "");

    return href;
}
