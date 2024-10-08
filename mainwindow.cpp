#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    //Исходное состояние виджетов
    ui->setupUi(this);
    ui->lb_statusConnect->setStyleSheet("color:red");
    ui->pb_request->setEnabled(false);

    /*
     * Выделим память под необходимые объекты. Все они наследники
     * QObject, поэтому воспользуемся иерархией.
    */

    dataDb = new DbData(this);
    dataBase = new DataBase(this);
    msg = new QMessageBox(this);

    //Установим размер вектора данных для подключения к БД
    dataForConnect.resize(NUM_DATA_FOR_CONNECT_TO_DB);

    /*
     * Добавим БД используя стандартный драйвер PSQL и зададим имя.
    */
    dataBase->AddDataBase(POSTGRE_DRIVER, DB_NAME);

    /*
     * Устанавливаем данные для подключениея к БД.
     * Поскольку метод небольшой используем лямбда-функцию.
     */
    connect(dataDb, &DbData::sig_sendData, this, [&](QVector<QString> receivData){
        dataForConnect = receivData;
    });

    /*
     * Соединяем сигналы, которые передают модели от БД с методами, которые отображают ответ в ui
     */
    connect(dataBase, &DataBase::sig_SendTableModel, this, QOverload<QSqlTableModel*>::of(&MainWindow::ScreenDataFromDB));
     connect(dataBase, &DataBase::sig_SendQueryModel, this, QOverload<QSqlQueryModel*>::of(&MainWindow::ScreenDataFromDB));

    /*
     *  Сигнал для подключения к БД
     */
    connect(dataBase, &DataBase::sig_SendStatusConnection, this, &MainWindow::ReceiveStatusConnectionToDB);

}

MainWindow::~MainWindow()
{
    delete ui;
}

/*!
 * @brief Слот отображает форму для ввода данных подключения к БД
 */
void MainWindow::on_act_addData_triggered()
{
    //Отобразим диалоговое окно. Какой метод нужно использовать?
    dataDb->show();
}

/*!
 * @brief Слот выполняет подключение к БД. И отображает ошибки.
 */

void MainWindow::on_act_connect_triggered()
{
    /*
     * Обработчик кнопки у нас должен подключаться и отключаться от БД.
     * Можно привязаться к надписи лейбла статуса. Если он равен
     * "Отключено" мы осуществляем подключение, если "Подключено" то
     * отключаемся
    */

    if(ui->lb_statusConnect->text() == "Отключено"){

       ui->lb_statusConnect->setText("Подключение");
       ui->lb_statusConnect->setStyleSheet("color : black");


       auto conn = [&]{dataBase->ConnectToDataBase(dataForConnect);};
       QtConcurrent::run(conn);

    }
    else{
        dataBase->DisconnectFromDataBase(DB_NAME);
        ui->lb_statusConnect->setText("Отключено");
        ui->act_connect->setText("Подключиться");
        ui->lb_statusConnect->setStyleSheet("color:red");
        ui->pb_request->setEnabled(false);
    }

}

/*!
 * \brief Обработчик кнопки "Получить"
 */
void MainWindow::on_pb_request_clicked()
{
    on_pb_clear_clicked();

    int type = ui->cb_category->currentIndex();
    QString category;

    switch (type)
    {
    case requestType::requestAllFilms:
        dataBase->GetTableModel();
        return;
    case requestType::requestComedy:
        category = "Comedy";
        break;
    case requestType::requestHorrors:
        category = "Horror";
        break;
    }

    request = QString("SELECT title, description FROM film f "
              "JOIN film_category fc on f.film_id = fc.film_id "
              "JOIN category c on c.category_id = fc.category_id WHERE c.name = '%1'").arg(category);

    dataBase->RequestToDB(request);
}

void MainWindow::ScreenDataFromDB(QSqlTableModel* model)
{
    ui->tw_films->setModel(model);
    ui->tw_films->hideColumn(0);
    for (int i = 3; i < model->columnCount(); ++i)
        ui->tw_films->hideColumn(i);
    ui->tw_films->resizeColumnsToContents();
    ui->tw_films->show();
}

void MainWindow::ScreenDataFromDB(QSqlQueryModel* model)
{
    ui->tw_films->setModel(model);
    ui->tw_films->resizeColumnsToContents();
    ui->tw_films->show();
}


/*!
 * \brief Метод изменяет стотояние формы в зависимости от статуса подключения к БД
 * \param status
 */
void MainWindow::ReceiveStatusConnectionToDB(bool status)
{
    if(status){
        ui->act_connect->setText("Отключиться");
        ui->lb_statusConnect->setText("Подключено к БД");
        ui->lb_statusConnect->setStyleSheet("color:green");
        ui->pb_request->setEnabled(true);
    }
    else{
        dataBase->DisconnectFromDataBase(DB_NAME);
        msg->setIcon(QMessageBox::Critical);
        msg->setText(dataBase->GetLastError().text());
        ui->lb_statusConnect->setText("Отключено");
        ui->lb_statusConnect->setStyleSheet("color:red");
        msg->exec();
    }

}

void MainWindow::on_pb_clear_clicked()
{
    if (ui->tw_films->model())
    {
//        QAbstractItemModel* model = ui->tw_films->model();
//        int rowCount = model->rowCount();
//        model->removeRows(0, rowCount);
        ui->tw_films->setModel(nullptr);
    }
}

