#ifndef UI_H
#define UI_H

 #include <QList>

class QPainter;
class GLWidget;
class Window;

class Ui
{
    friend class Window;
public:
    Ui(GLWidget* glW);
    void Draw(QPainter* painter);
    void convertMousePos(int x, int y);
    bool mouseClick();
private:
    GLWidget* glW;
    qreal mouseX,mouseY;
    QList<Window*> windows;
    Window* selectedWindow;
};

#endif // UI_H
