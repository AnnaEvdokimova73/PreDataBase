#include "database.h"

DataBase::DataBase(QObject *parent)
    : QObject{parent}
{

    dataBase = new QSqlDatabase();
    queryModel = new QSqlQueryModel();

}

DataBase::~DataBase()
{
    delete queryModel;
    delete tableModel;
    delete dataBase;
}

/*!
 * \brief Метод добавляет БД к экземпляру класса QSqlDataBase
 * \param driver драйвер БД
 * \param nameDB имя БД (Если отсутствует Qt задает имя по умолчанию)
 */
void DataBase::AddDataBase(QString driver, QString nameDB)
{

    *dataBase = QSqlDatabase::addDatabase(driver, nameDB);

}

/*!
 * \brief Метод подключается к БД
 * \param для удобства передаем контейнер с данными необходимыми для подключения
 * \return возвращает тип ошибки
 */
void DataBase::ConnectToDataBase(QVector<QString> data)
{

    dataBase->setHostName(data[hostName]);
    dataBase->setDatabaseName(data[dbName]);
    dataBase->setUserName(data[login]);
    dataBase->setPassword(data[pass]);
    dataBase->setPort(data[port].toInt());

    bool status;
    status = dataBase->open();
    emit sig_SendStatusConnection(status);

    tableModel = new QSqlTableModel(nullptr, *dataBase);

}


/*!
 * \brief Метод производит отключение от БД
 * \param Имя БД
 */
void DataBase::DisconnectFromDataBase(QString nameDb)
{
    *dataBase = QSqlDatabase::database(nameDb);
    dataBase->close();

}
/*!
 * \brief Метод формирует запрос к БД.
 * \param request - SQL запрос
 * \return
 */
void DataBase::RequestToDB(QString request)
{
//    queryModel = new QSqlQueryModel();
    queryModel->setQuery(request, *dataBase);
    queryModel->setHeaderData(0, Qt::Horizontal, tr("Film name"));
    queryModel->setHeaderData(1, Qt::Horizontal, tr("Description"));

    emit sig_SendQueryModel(queryModel);
}

void DataBase::GetTableModel()
{
//    tableModel = new QSqlTableModel(nullptr, *dataBase);
    tableModel->setTable("film");
    tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    tableModel->select();
    tableModel->setHeaderData(1, Qt::Horizontal, tr("Film name"));
    tableModel->setHeaderData(2, Qt::Horizontal, tr("Description"));

    emit sig_SendTableModel(tableModel);
}

/*!
 * @brief Метод возвращает последнюю ошибку БД
 */
QSqlError DataBase::GetLastError()
{
    return dataBase->lastError();
}
