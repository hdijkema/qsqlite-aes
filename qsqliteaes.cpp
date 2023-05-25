#include "qsqliteaes.h"

#include <QtGlobal>
#include <QString>
#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "qsql_sqlite_p.h"

QSQLiteAES::QSQLiteAES(QObject *parent)
    : QSqlDriverPlugin(parent)
{
}

QSqlDriver *QSQLiteAES::create(const QString &name)
{
    QString qt_version(QLatin1String(QT_VERSION_STR));
    QString myname(QLatin1String("QSQLiteAES_%1"));

    QString my_version_name(myname.arg(qt_version.replace(QLatin1String("."), QLatin1String("_"))));

    if (name == my_version_name) {
        QSQLiteDriver* driver = new QSQLiteDriver();
        return driver;
    }
    return nullptr;
}

