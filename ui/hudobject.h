#ifndef HUDOBJECT_H
#define HUDOBJECT_H

#include <QObject>
class QPainter;
class Ui;

class HudObject : public QObject
{
    Q_OBJECT
public:
    HudObject();
    virtual void draw(QPainter* painter);
    virtual bool click();
    virtual void clickRelease();
    virtual bool hovers();
    virtual void resize();
    virtual void move(int x, int y);
    virtual void hide();
    virtual void show();

    Ui* ui;
};

#endif // HUDOBJECT_H
