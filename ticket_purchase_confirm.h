#ifndef TICKET_PURCHASE_CONFIRM_H
#define TICKET_PURCHASE_CONFIRM_H
#include "ticket_purchase.h"
#include <QWidget>

namespace Ui {
class ticket_purchase_confirm;
}

class ticket_purchase_confirm : public QWidget
{
    Q_OBJECT

public:
    explicit ticket_purchase_confirm(QWidget *parent = nullptr,Ticket_Purchase *parent_ticket=nullptr
            ,QString moneyStr="",QString price="",QString classType="",QString FromOrder="0",int offset=0);
    ~ticket_purchase_confirm();
    void AutoConfirm();


private slots:
    void on_pushButton_cancel_clicked();

    void on_pushButton_confirm_clicked();

private:
    Ui::ticket_purchase_confirm *ui;
    Ticket_Purchase *parent_ticket;
    QString moneyStr;
    QString price;
    QString classType;
    QString FromOrder;
    int offset;
};

#endif // TICKET_PURCHASE_CONFIRM_H
