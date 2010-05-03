#ifndef HUDOBJECT_H
#define HUDOBJECT_H

class QPainter;


class HudObject
{
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
};

#endif // HUDOBJECT_H
