#ifndef RADAR_H
#define RADAR_H

#include "hudobject.h"
#include "glwidget.h"

class Radar : public HudObject
{
    Q_OBJECT
public:
    Radar(Ui* ui);

    //overloaded
    void draw(QPainter* painter);
    bool click();
    void clickRelease();
    void hide();
    void show();
    void move(int x, int y);
    bool hovers();
    void resize();

public slots:
    void changeState();

private:
    bool hoversMap();
    void drawMap(QPainter* painter);
    bool clickMap();
    bool hidden;
};

#endif // RADAR_H
