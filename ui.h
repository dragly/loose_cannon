#ifndef UI_H
#define UI_H

 #include <QList>

class GLWidget;
class Window;

class Ui
{
    friend class Window;
public:
    Ui(GLWidget* glW);
    void Draw();
    void convertMousePos(int x, int y);
    bool mouseClick();
private:
    GLWidget* glW;
    double mouseX,mouseY;
    QList<Window*> windows;
    Window* selectedWindow;
};

#endif // UI_H
