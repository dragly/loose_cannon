#ifndef WINDOW_H
#define WINDOW_H

#include <QList>
#include <QtGui/qvector3d.h>
#include <QSize>

class Ui;
class QPainter;
class QColor;
class Controller;

class Window {

public:

    friend class Ui;
    friend class Controller;
    friend class Cbutton;

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
    void draw(QPainter* painter);
    bool click();
    void hide();
    void show();
    void  addController(Controller* controller);
    bool hovers();

private:
    void drawBackground(QPainter* painter);

    Controller* selectedController;

    bool titlebar; //if no, then don't draw a titlebar :P
    QString title;

    QList<Controller*> controllers;

    bool hidden; //window is invisible yet not destructed.

    QPoint pos;
    QSize size;
    Ui* ui;
    bool projected;
    QVector3D* world; //IMPORTANT: This is a pointer, so it will crash if the location disappears, could do an entity pointer instead?

    static const QColor ColorBackground;
    static const QColor ColorBorder;
    static const QColor ColorText;
    static const QColor ColorHighlight;
    static const QColor ColorNone;

};

#endif // WINDOW_H
