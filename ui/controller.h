#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QPoint>
#include <QSize>
class QSize;
class QPoint;
class Window;
class QPainter;

class Controller : public QObject {
    Q_OBJECT

public:
    Controller(Window* parent, QPointF relativePos, QSizeF size);

    virtual bool click();
    virtual void move();
    virtual void draw(QPainter *painter);
protected:
    QPoint pos;
    QSize size;
    Window* parent;
};

#endif // CONTROLLER_H
