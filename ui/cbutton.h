#ifndef CBUTTON_H
#define CBUTTON_H

#include  "controller.h"

class QPoint;
class QSize;
class Window;

class Cbutton : public Controller
{
    Q_OBJECT
public:
    Cbutton(Window* parent, QPointF pos, QString text);
    bool click();
    void move();
    void draw(QPainter *painter);
signals:
    void btnClicked();

private:
    QString text;

};

#endif // CBUTTON_H
