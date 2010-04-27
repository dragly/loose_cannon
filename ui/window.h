#ifndef WINDOW_H
#define WINDOW_H

#include <QList>
#include <QtGui/qvector3d.h>
#include <QSize>

class Element;
class Ui;
class QPainter;

class Window
{

public:
    friend class Ui;

    //bind the given side of the window to the given side of the screen, offset with the specified x and y coords
    enum Alignments {
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

    Window(Ui* ui, qreal x,qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool projected, QVector3D* world, bool titlebar = false, QString title = NULL);
    Window(Ui* ui, qreal x,qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool titlebar = false, QString title = NULL);
    void Draw(QPainter* painter);
    void Click();
    bool Hovers();

private:
    void drawBackground();
    void drawElements();

    bool titlebar; //if no, then don't draw a titlebar :P
    QString title;

    Alignments alignment;
    QList<Element*> elements;

    QPoint pos;
    QSize size;
    Ui* ui;
    bool projected;
    QVector3D* world; //IMPORTANT: This is a pointer, so it will crash if the location disappears, could do an entity pointer instead?

};

#endif // WINDOW_H