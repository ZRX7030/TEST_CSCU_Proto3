#ifndef MODULETERM_H
#define MODULETERM_H

#include <QWidget>

namespace Ui {
class ModuleTerm;
}

class ModuleTerm : public QWidget
{
    Q_OBJECT

public:
    explicit ModuleTerm(QWidget *parent = 0);
    ~ModuleTerm();

private:
    Ui::ModuleTerm *ui;
};

#endif // MODULETERM_H
