#ifndef WINDOW_H
#define WINDOW_H

#include <QList>
#include <QtGui/qvector3d.h>
#include <QSize>
#include "hudobject.h"

class Ui;
class QPainter;
class QColor;
class Controller;
class HudObject;

class Window : public HudObject {

public:

    friend class Ui;
    friend class Controller;
    friend class Cbutton;

    static const QColor ColorBackground;
    static const QColor ColorBorder;
    static const QColor ColorText;
    static const QColor ColorHighlight;
    static const QColor ColorNone;


    //bind the given side of the window to the given side of the screen, offset with the specified x and y coords
    enum Alignments {
        TopLeft,
        TopCenter,
        TopRight,
        CenterLeft,
        Center,
        CenterRight,
        BottomLeft,
        BottomCenter,
        BottomRight,
    };
    Window(Ui* ui, qreal x, qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool projected, QVector3D* world, bool titlebar = false, QString title = NULL); 
    Window(Ui* ui, qreal x, qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool titlebar = false, QString title = NULL);

    void init(Ui* ui, qreal x, qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool projected, QVector3D* world, bool titlebar = false, QString title = NULL);

    //overloaded
    void draw(QPainter* painter);
    bool click();
    void clickRelease();
    void hide();
    void show();
    void move(int x, int y);
    bool hovers();
    void resize();

    void addController(Controller* controller);
    QVector3D* world; //IMPORTANT: This is a pointer, so it will crash if the location disappears, could do an entity pointer instead?

private:
    void drawBackground(QPainter* painter);

    //Controller* selectedController;

    bool titlebar; //if no, then don't draw a titlebar :P
    QString title;

    QList<Controller*> controllers;

    bool hidden; //window is invisible yet not destructed.

    Alignments alignment;
    QPoint pos;
    QSize size;
    QSizeF relativeSize;
    QPoint relativePos;

    bool projected;
};

#endif // WINDOW_H
