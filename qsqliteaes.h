#ifndef QSQLITEAES_H
#define QSQLITEAES_H

#include <QSqlDriverPlugin>

class QSQLiteAES : public QSqlDriverPlugin
{
    Q_OBJECT
#if (QT_VERSION == QT_VERSION_CHECK(5, 15, 2))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "qsqlite-aes2_5_15_2.json")
#else
#if (QT_VERSION == QT_VERSION_CHECK(6, 5, 1))
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QSqlDriverFactoryInterface" FILE "qsqlite-aes2_6_5_1.json")
#else
#error QT_VERSION: De huidige Qt versie is niet ondersteund door qsqlite-aes2. Zorg voor de juiste .json file.
#endif
#endif

public:
    explicit QSQLiteAES(QObject *parent = nullptr);

private:
    QSqlDriver *create(const QString &key) override;
};

#endif // QSQLITEAES_H
