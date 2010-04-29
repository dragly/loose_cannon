#ifndef UI_H
#define UI_H

 #include <QList>

class QPainter;
class GLWidget;
class Window;
class Controller;

class Ui
{
    friend class Window;
    friend class Controller;
    friend class Cbutton;
public:
    Ui(GLWidget* glW);
    void draw(QPainter* painter);
    void convertMousePos(int x, int y);
    bool mouseClick();
    void mouseRelease();
    void addWindow(Window* window);
    bool isMouseDown();
private:
    GLWidget* glW;
    qreal mouseX,mouseY;
    QList<Window*> windows;
    Window* selectedWindow;
    bool mouseDown;
};

#endif // UI_H
