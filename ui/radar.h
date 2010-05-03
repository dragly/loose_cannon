#ifndef RADAR_H
#define RADAR_H

#include "hudobject.h"
#include "glwidget.h"

class Radar : public HudObject
{
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

private:
    bool hoversIcon();
    bool hoversMap();
    void drawIcon(QPainter* painter);
    void drawMap(QPainter* painter);
    bool clickMap();
    bool hidden;
    bool btnDown;
    Ui* ui;
};

#endif // RADAR_H
