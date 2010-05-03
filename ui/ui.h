#ifndef UI_H
#define UI_H

 #include <QList>
#include <QPoint>

class QPainter;
class GLWidget;
class Window;
class Controller;
class HudObject;
class HudIcon;

class Ui
{
    friend class Window;
    friend class Radar;
    friend class Controller;
    friend class Cbutton;
    friend class HudIcon;
public:
    Ui(GLWidget* glW);

    void draw(QPainter* painter);
    void convertMousePos(int x, int y);
    bool mouseClick();
    void mouseRelease();
    void move();

    void addHudObject(HudObject* object);
    bool isMouseDown();
    void moveToFront(HudObject *object);
    void drawMap(QPainter* painter);
    bool clickMap();
private:
    GLWidget* glW;
    qreal mouseX,mouseY,lastMouseX,lastMouseY;
    QList<HudObject*> hudObjects;
    HudObject* selectedHudObject;
    bool mouseDown;
};

#endif // UI_H
