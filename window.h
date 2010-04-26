#ifndef WINDOW_H
#define WINDOW_H

#include <QList>
#include <QtGui/qvector3d.h>

class Element;

class Window
{

public:

    //bind the given side of the window to the given side of the screen, offset with the specified x and y coords
    enum Alignment {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomRight,
        BottomCenter,
        BottomLeft,
    };

    Window(double x,double y, double sizeX, double sizeY, Alignment alignment, bool projected, QVector3D* world, bool titlebar = false, QString title = NULL);
    Window(double x,double y, double sizeX, double sizeY, Alignment alignment, bool titlebar = false, QString title = NULL);
    void Draw();
    void Click();
    bool Hovers();

private:
    void drawBackground();
    void drawElements();

    bool titlebar; //if no, then don't draw a titlebar :P
    QString title;

    Alignment alignment;
    double x,y,z,sizeX,sizeY; //z is only used in the case of projections.
    QList<Element*> elements;

    bool projected;
    QVector3D* world; //IMPORTANT: This is a pointer, so it will crash if the location disappears, could do an entity pointer instead?

};

#endif // WINDOW_H
