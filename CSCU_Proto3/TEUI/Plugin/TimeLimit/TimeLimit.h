#ifndef TIMELIMIT_H
#define TIMELIMIT_H

#include <QWidget>

namespace Ui {
class TimeLimit;
}

class TimeLimit : public QWidget
{
    Q_OBJECT

public:
    explicit TimeLimit(QWidget *parent = 0);
    ~TimeLimit();
     void startDownCout(int seconds);
     void stopDownCout();
signals:
     void timeout();

private:
    Ui::TimeLimit *ui;
    int secondCount;
    QTimer *timer ;

private slots:
    void timerDownDate();
};

#endif // TIMELIMIT_H
