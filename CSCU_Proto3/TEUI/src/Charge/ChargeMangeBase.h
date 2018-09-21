#ifndef CHARGEMANGEBASE_H
#define CHARGEMANGEBASE_H

#include <QWidget>
#include <QVariant>

class ChargeMangeBase : public QWidget
{
    Q_OBJECT
public:
    explicit ChargeMangeBase(QWidget *parent = 0);
    void switchToBase(QString result, int type, QVariant vsr);

signals:

public slots:

signals:

    void sigBackToMain(int oldStatus, int newStatus, QVariant var);
};

#endif // CHARGEMANGEBASE_H
