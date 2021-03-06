#include "flight_inquiry_citys_and_date.h"
#include "ui_flight_inquiry_citys_and_date.h"
#include "flight_inquiry.h"
#include "mainclientwindow.h"
#include "account_and_orders.h"
#include "login.h"
#include "ticket_purchase.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStringList>
#include <QSettings>
#include <QDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QFile>
#include <QScroller>
extern account_and_orders * acct; //只有当用户进入到账户界面后，也即UserID!=""时，可以有效使用

flight_inquiry_citys_and_date::flight_inquiry_citys_and_date(
    QWidget *parent,QString UserID,QString Password, QString Mode
        ,QString Name,QString FromOrder,QString StartCity,QString EndCity)
    : QWidget(parent), ui(new Ui::flight_inquiry_citys_and_date) {
    ui->setupUi(this);
    QFile styleFile(":/style.qss");//使用qss文件
    styleFile.open(QIODevice::ReadOnly);//只读
    QString setStyleSheet(styleFile.readAll());//读取所有的
    ui->Flights->setStyleSheet(setStyleSheet);//设置样式
    QScroller::grabGesture(ui->Flights, QScroller::TouchGesture);
    this->UserID = UserID;
    this->Password = Password;
    this->Mode = Mode;
    this->Name = Name;
    this->FromOrder = FromOrder;

    if (UserID == "") { //在之前的flight_inquiry已经对账户信息进行了筛选，此处不再重复
        ui->UserStatus->setText(tr("User Information: Not logged in"));
    } else {
        ui->UserStatus->setText(tr("User Information: ")+this->Name);
        ui->pushButton_4->hide();
    }

    //只查询起降城市，不需要起飞日期
    if(Mode == 'C') {
        ui->label_3->hide();
        ui->dateEdit->hide();
    }

    ui->label->setText(tr("Departure City"));
    ui->DepatureCity->setEchoMode(QLineEdit::Normal);
    ui->DepatureCity->setPlaceholderText(tr("Please Enter Your Departure City."));

    ui->label_2->setText(tr("Arrival City"));
    ui->ArrivalCity->setEchoMode(QLineEdit::Normal);
    ui->ArrivalCity->setPlaceholderText(tr("Please Enter Your Arrival City."));

    ui->label_3->setText(tr("Departure Date"));
    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit->setMinimumDate(QDate::currentDate());

    ui->pushButton->setText(tr("Confirm"));
    ui->pushButton_2->setText(tr("Back"));
    ui->pushButton_3->setText(tr("Cancel"));
    ui->label_4->setText(tr("Flight Information:"));
    ui->pushButton_4->setText(tr("Log In"));
    //layout()->setSizeConstraint(QLayout::SetFixedSize);

    if(FromOrder == "1"){ //说明该页面父页面来自于订单，属于改期的任务，需要固定出发的城市
        ui->pushButton->setText(tr("Refresh"));
        ui->pushButton_2->hide();
        ui->DepatureCity->setText(StartCity);
        ui->DepatureCity->setDisabled(true);
        ui->ArrivalCity->setText(EndCity);
        ui->ArrivalCity->setDisabled(true);
        this->on_pushButton_clicked();
    }

}

flight_inquiry_citys_and_date::~flight_inquiry_citys_and_date() {
    delete ui;
}

//Btn: Back
void flight_inquiry_citys_and_date::on_pushButton_2_clicked() {
    flight_inquiry * flt_inq = new flight_inquiry(nullptr,this->UserID,this->Password,this->Name);
    flt_inq->show();
    this->close();
}

//Btn:Cancel
void flight_inquiry_citys_and_date::on_pushButton_3_clicked() {
    qDebug()<<"You decide to quit."<<endl;
    if(this->FromOrder=="1"){
        acct->setStatus(1);
    }
    this->close();
}

//Btn:Confirm- for searching
void flight_inquiry_citys_and_date::on_pushButton_clicked() {
    qDebug()<<Mode<<endl;
    QString dep_city = ui->DepatureCity->text();
    QString arv_city = ui->ArrivalCity->text();
    QString dep_date = ui->dateEdit->text();

    //允许只按照起降城市查询；
    if (dep_city == ""||arv_city == "") {
        QMessageBox::critical(this,tr("Critical"),tr("Incomplete input."));
        return;
    }

    if (this->Mode == "CD" && dep_date=="") {
        QMessageBox::critical(this,tr("Critical"),tr("Incomplete input."));
        return;
    }

    //航班查询
    QString sql;
    if (this->Mode == "CD") {
        sql = QString("CALL flight_query_CD('%1','%2'"
                      ",'%3')").arg(dep_city).arg(arv_city).arg(dep_date);

    } else {
        sql = QString("CALL flight_query_C('%1','%2')").arg(dep_city).arg(arv_city);
    }
    // 查询结果的日期和时间是在NOW()的之后十分钟以及以后

    qDebug()<<sql<<endl;

    MyQueryMode *model = new MyQueryMode();
    model->setQuery(sql);
    model->setHeaderData(0,Qt::Horizontal,QString::fromUtf8(tr("Departure Date").toUtf8()));
    model->setHeaderData(1,Qt::Horizontal,QString::fromUtf8(tr("Flight ID").toUtf8()));
    model->setHeaderData(2,Qt::Horizontal,QString::fromUtf8(tr("Schedule").toUtf8()));
    model->setHeaderData(3,Qt::Horizontal,QString::fromUtf8(tr("Departure Airport ID").toUtf8()));
    model->setHeaderData(4,Qt::Horizontal,QString::fromUtf8(tr("Departure Airport").toUtf8()));
    model->setHeaderData(5,Qt::Horizontal,QString::fromUtf8(tr("Departure City").toUtf8()));
    model->setHeaderData(6,Qt::Horizontal,QString::fromUtf8(tr("Departure Time").toUtf8()));
    model->setHeaderData(7,Qt::Horizontal,QString::fromUtf8(tr("Departure Positon").toUtf8()));
    model->setHeaderData(8,Qt::Horizontal,QString::fromUtf8(tr("Arrival Airport ID").toUtf8()));
    model->setHeaderData(9,Qt::Horizontal,QString::fromUtf8(tr("Arrival Airport").toUtf8()));
    model->setHeaderData(10,Qt::Horizontal,QString::fromUtf8(tr("Arrival City").toUtf8()));
    model->setHeaderData(11,Qt::Horizontal,QString::fromUtf8(tr("Arrival Time").toUtf8()));
    model->setHeaderData(12,Qt::Horizontal,QString::fromUtf8(tr("Arrival Positon").toUtf8()));
    model->insertColumn(13); // 插入购买列
    model->setHeaderData(13,Qt::Horizontal,QString::fromUtf8(tr("Booking").toUtf8()));
    ui->Flights->setModel(model);
    ui->Flights->resizeColumnsToContents();

    //还需要实现每一行的"购票"的按钮；

}

QVariant MyQueryMode::data(const QModelIndex &item, int role) const {
    QVariant value = QSqlQueryModel::data(item,role);
    if (role == Qt::BackgroundColorRole) {
        if(item.column()==13)
            return QVariant::fromValue(QColor(225,225,225));
    }
    if (role == Qt::DisplayRole) {
        if(item.column()==7 || item.column()==12) {
            if(QSqlQueryModel::data(item).toInt()==0)
                return QVariant::fromValue(QString(tr("Of Departure")));
            else if(QSqlQueryModel::data(item).toInt()==-1)
                return QVariant::fromValue(QString(tr("Of Arrival")));
            else {
                QString order = QSqlQueryModel::data(item).toString();
                QString info = tr("No.") + order + tr(" Transfer");
                return QVariant::fromValue(info);
            }
        }
        if(item.column()==13)
            return QVariant::fromValue(tr("Booking"));

        if(item.column()==6 || item.column()==11){
            QString timeDisplay =  QSqlQueryModel::data(item).toString();
            int start = timeDisplay.mid(0,2).toInt();
            int flag = 0;
            if(start>=24){
                start -= 24;
                flag = 1;
            }
            if(flag){
                QString startStr;
                if(start>=10){
                    startStr = QString("%1").arg(start);
                }else{
                    startStr = QString("0%1").arg(start);
                }
                if(item.column()==6){
                    timeDisplay = "* "+startStr+timeDisplay.mid(2);
                }else{//item.column()==12
                    QModelIndex index_depTime = this->index(item.row(),6);
                    QString str_depTime = index_depTime.data().toString();
                    if(str_depTime.mid(0,1)=="*")
                        timeDisplay = startStr+timeDisplay.mid(2);
                    else
                        timeDisplay = "(+1)"+startStr+timeDisplay.mid(2);
                }
            }

            return QVariant::fromValue(timeDisplay);
        }
        if(item.column()==0){
            QDate dateDisplay = QSqlQueryModel::data(item).toDate();
            QModelIndex index_depTime = this->index(item.row(),6);
            QString str_depTime = index_depTime.data().toString();
            if(str_depTime.mid(0,2).toInt()>=24){ //说明日期要做改变
                dateDisplay = dateDisplay.addDays(1);
            }
            QString str_dateDisplay = dateDisplay.toString("yyyy-MM-dd");
            QVariant var_dateDisplay(str_dateDisplay);
            return QVariant::fromValue(var_dateDisplay);
            qDebug()<<value.toString()<<endl;
        }

    }
    return value;
}


void flight_inquiry_citys_and_date::on_Flights_clicked(const QModelIndex &index) {
    if (index.isValid()&&index.column()==13) { //点击订票键，打开相关的UI界面，显示价格并进行购票操作
        //如果没有登陆，则提示登陆账户后再买票，并结束该界面，进入到登陆界面
        //如果登陆了，则打开购票信息框
        qDebug()<<"你刚刚点击了订票按钮"<<endl;
        if(this->UserID=="") {
            //说明是以游客形式登陆的，提示其先进行账户登陆
            QMessageBox::information(this,tr("Hint:"),tr("Before booking, You need to log in first."));
            return;
        }

        int row = index.row(); // 返回点击单元格的所在行数，从而根据行来提取相关航班的信息。
        QAbstractItemModel* model = ui->Flights->model();
        QString dep_date = model->data(model->index(row,0)).toString();
        QString fligh_id = model->data(model->index(row,1)).toString();
        QString schedule = model->data(model->index(row,2)).toString();
        QString dep_airportName = model->data(model->index(row,4)).toString();
        QString dep_city = model->data(model->index(row,5)).toString();
        QString dep_time = model->data(model->index(row,6)).toString();
        QString arv_airportName = model->data(model->index(row,9)).toString();
        QString arv_city = model->data(model->index(row,10)).toString();
        QString arv_time = model->data(model->index(row,11)).toString();
        QString order_start = model->data(model->index(row,7)).toString();
        QString order_end = model->data(model->index(row,12)).toString();


        if(order_start==tr("Of Departure")) order_start="0";
        else if(order_start==tr("Of Arrival")) order_start="-1";
        else order_start=order_start.mid(1,1);

        if(order_end==tr("Of Departure")) order_end="0";
        else if(order_end==tr("Of Arrival")) order_end="-1";
        else order_end=order_end.mid(1,1);

        //检查该用户是否已经购买过同一趟航班。如果是，则提示用户不能重复购买，并取消预定操作
        QString sql_check_doublebooking = QString("SELECT COUNT(*) FROM ticket WHERE flight_id='%1' "
                                          "AND ID='%2' AND CAST(departure_datetime AS date)='%3' AND ticket_id NOT IN "
                                          "(SELECT ticket_id FROM ticket_refund ) ")
                                          .arg(fligh_id).arg(acct->getUserID()).arg(dep_date);

        qDebug()<<sql_check_doublebooking<<endl;
        QSqlQuery *query_check_doublebooking = new QSqlQuery();
        query_check_doublebooking->exec(sql_check_doublebooking);
        query_check_doublebooking->next();//查询成功，则表明此用户此前曾购买过同一趟的飞机，则此时提示已购买同一趟飞机，并禁止购买
        if(query_check_doublebooking->value(0).toInt()) {
            QMessageBox::information(this,tr("Hint:"),tr("You have already booked this flight. Please choose another flight."));
            return;
        }
        QVariant dep_date_correct_qvar(dep_date);
        QDate dep_date_correct_qdate = dep_date_correct_qvar.toDate();
        if(dep_time.mid(0,1)=="*"){ //说明实际用于余票查询的日期应该为当前日期-1
            dep_date_correct_qdate = dep_date_correct_qdate.addDays(-1);
        }
        QString dep_date_correct_str = dep_date_correct_qdate.toString("yyyy-MM-dd");



        //根据所截取的信息来查询该趟航班是否有余票（在确认购买时仍需要查询是否有余票，以保持数据的一致性）
        QString sql_remaining_tickets_business = QString("CALL remaining_tickets_num("
                "'%1','%2',%3,%4,0)").arg(fligh_id).arg(dep_date_correct_str).arg(order_start).arg(order_end);
        QString sql_remaining_tickets_economy = QString("CALL remaining_tickets_num("
                                                "'%1','%2',%3,%4,1)").arg(fligh_id).arg(dep_date_correct_str).arg(order_start).arg(order_end);
        qDebug()<<sql_remaining_tickets_business<<endl;
        qDebug()<<sql_remaining_tickets_economy<<endl;

        int business_remaining_num=0;
        QSqlQuery *query_remaining_tickets_business = new QSqlQuery();
        query_remaining_tickets_business->exec(sql_remaining_tickets_business);
        if(query_remaining_tickets_business->next()) {
            business_remaining_num = query_remaining_tickets_business->value(0).toInt();
        }
        int economy_remaining_num=0;
        QSqlQuery *query_remaining_tickets_econuomy = new QSqlQuery();
        query_remaining_tickets_econuomy->exec(sql_remaining_tickets_economy);
        if(query_remaining_tickets_econuomy->next()) {
            economy_remaining_num = query_remaining_tickets_econuomy->value(0).toInt();
        }

        qDebug()<<"公务舱剩余座位数:"<<business_remaining_num<<endl;
        qDebug()<<"经济舱剩余座位数:"<<economy_remaining_num<<endl;

        if (business_remaining_num==0 && economy_remaining_num==0) {
            QMessageBox::information(this,tr("Hint:"),tr("There are no tickets left on this flight. Please choose another flight."));
            return;
        }
        qDebug()<<"仍有余票，允许购买"<<endl;
        //否则，还剩余有机票可以购买，则进入到购票界面。
        int offset=0;
        if(dep_date_correct_str!=dep_date) offset=1;
        if(this->FromOrder=="0"){
            Ticket_Purchase *purchase_interface = new Ticket_Purchase(nullptr,this,nullptr,dep_date,fligh_id,schedule,dep_airportName,dep_city,dep_time,
                    arv_airportName,arv_city,arv_time,order_start,order_end,"0",offset);
            purchase_interface->show();
        }else{
            Ticket_Purchase *purchase_interface = new Ticket_Purchase(nullptr,this,nullptr,dep_date,fligh_id,schedule,dep_airportName,dep_city,dep_time,
                    arv_airportName,arv_city,arv_time,order_start,order_end,"1",offset);
            purchase_interface->show();
        }
    }
}

//Btn: log in
void flight_inquiry_citys_and_date::on_pushButton_4_clicked() {
    login *lgin = new login();
    lgin->show();
    QApplication::processEvents();
    this->close();
}
void flight_inquiry_citys_and_date::setDepartureCity(QString dep) {
    ui->DepatureCity->setText(dep);
}
void flight_inquiry_citys_and_date::setArrivalCity(QString arr) {
    ui->ArrivalCity->setText(arr);
}
void flight_inquiry_citys_and_date::setDate(QDate dat) {
    ui->dateEdit->setDate(dat);
}
