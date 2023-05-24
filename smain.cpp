/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGlobal>
#include <QString>
#include <qsqldriverplugin.h>
#include <qstringlist.h>
#include "qsql_sqlite_p.h"

QT_BEGIN_NAMESPACE

class QSQLiteAESPlugin : public QSqlDriverPlugin
{
    Q_OBJECT
    //Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes.json")
#if (QT_VERSION == QT_VERSION_CHECK(5, 12, 3))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_5_12_3.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(5, 12, 5))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_5_12_5.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(5, 14, 0))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_5_14_0.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(5, 13, 1))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_5_13_1.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(5, 14, 2))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_5_14_2.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(5, 15, 0))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_5_15_0.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(5, 15, 2))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_5_15_2.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(6, 2, 3))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_6_2_3.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(6, 2, 4))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_6_2_4.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(6, 3, 1))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_6_3_1.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(6, 4, 0))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_6_4_0.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(6, 4, 3))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "sqlite-aes_6_4_3.json")
#else
#error QT_VERSION: De huidige Qt versie is niet ondersteund door qsqlite-aes. Zorg voor de juiste .json file.
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

public:
    QSQLiteAESPlugin();

    QSqlDriver* create(const QString &) Q_DECL_OVERRIDE;
};

QSQLiteAESPlugin::QSQLiteAESPlugin()
    : QSqlDriverPlugin()
{
}

QSqlDriver* QSQLiteAESPlugin::create(const QString &name)
{
    QString qt_version(QLatin1String(QT_VERSION_STR));
    QString myname(QLatin1String("QSQLiteAES_%1"));
    QString my_version_name(myname.arg(qt_version.replace(QLatin1String("."), QLatin1String("_"))));
    //QString myname = QString(QLatin1String("QSQLiteAES_%1")).arg(QString(QT_VERSION_STR).replace(QLatin1String("."), QLatin1String("_")));
    //if (name == QLatin1String(myname)) {
    if (name == my_version_name) {
        QSQLiteDriver* driver = new QSQLiteDriver();
        return driver;
    }
    return nullptr;
}

QT_END_NAMESPACE

#include "smain.moc"
