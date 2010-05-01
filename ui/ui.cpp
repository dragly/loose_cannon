#include "ui.h"
#include "glwidget.h"
#include "window.h"


Ui::Ui(GLWidget* glW)
{
    this->glW=glW;
    mouseDown=false;
}

void Ui::addWindow(Window* window) {
    windows.prepend(window);
}

void Ui::moveToFront(Window *window) {
    //NB: will crash if 'window' is not part of the list.
    windows.move(windows.indexOf(window),0);
    selectedWindow=window;
}

void Ui::convertMousePos(int x, int y) {
    // scale off of the height
    mouseX = x/*/glW->height()*/;
    mouseY = y;
}

void Ui::draw(QPainter* painter) {

    //draw the map
    drawMap(painter);

    for ( int i = windows.size()-1;i>=0;i--) { // loop trough all windows in reverse order
        windows.at(i)->draw(painter);
    }
}
bool Ui::isMouseDown() {
    return mouseDown;
}

//Ui::mouseMove etc..
void Ui::mouseRelease() {
    mouseDown=false;
    if (selectedWindow != NULL && selectedWindow->hovers()) {
        selectedWindow->click();
    }
}

bool Ui::mouseClick() {
    mouseDown=true;

    for (int i=0; i<windows.size(); i++) {
        Window* window = windows.at(i);
        if (window->hovers()) {
            windows.move(i,0);
            selectedWindow=window; //this one should always be at slot 0 to be drawn on top.
            return true;
        }
    }

    //if the last mouse press was in a menu but this one wasn't, hide all windows
    if (selectedWindow != NULL) {
        for (int i=0; i<windows.size(); i++) {
            windows.at(i)->hide();
        }
    }

    selectedWindow=NULL;


    return false;

}

void Ui::drawMap(QPainter* painter) {

    //these should be fetched form glW
    const qreal MAPSIZE = 30; //copied from glwidgets mapsize, which (confusingly) is only half of the actual mapsize.
    const qreal MapX = -MAPSIZE;
    const qreal MapY = -MAPSIZE;
    const qreal MapWidth= MAPSIZE*2;
    const qreal MapHeight= MAPSIZE*2;

    //temp vars
    const qreal Sqrt2 = 1.414213562373095;
    const qreal MapSize=0.25; //size of the UiMap
    const qreal XPos = 0.05; //from the edge
    const qreal YPos = 0.05;
    const qreal DotSize = 0.0001; //factor, small because it is evaluted after the scaling

    //if (dont draw map) return;

    int height = glW->height();

    QPoint mapPos((qreal) glW->width() - glW->height()*MapSize*Sqrt2/2/*-0.025*height*/,0/*-0.025*height*/);
    qreal mapSize= MapSize*height;
    qreal dotSize = DotSize*MapHeight*height*MapSize;

    //painter->setViewTransformEnabled();
    painter->translate(mapPos);
    painter->rotate(45);

    //draw all surroundings and the map picture
    QRectF map(0,0,mapSize,mapSize);

    painter->setClipRect(map); //clip stuff outside the map (mainly the camerasquare)

    painter->setPen(Window::ColorNone);
    painter->setBrush(Window::ColorBackground);
    painter->drawRect(map);

    //convert to world coords
    painter->scale(mapSize/MapWidth, mapSize/MapHeight);
    painter->translate(- dotSize/2 - MapX,-dotSize/2 - MapY); //reposition the dots

    painter->setPen(Window::ColorNone);

    //draw buildings
    painter->setBrush(Qt::darkGreen);
    foreach(Entity* ent, glW->buildings) {
       painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),dotSize,dotSize));
    }

    //draw friends
    painter->setBrush(Qt::green);
    foreach(Entity* ent, glW->units) {
        painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),dotSize,dotSize));
    }

    //draw enemies
    painter->setBrush(Qt::red);
    foreach(Entity* ent, glW->enemies) {
        painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),dotSize,dotSize));
    }

    //draw the camera square
    QPen pen(Qt::white,0,Qt::SolidLine,Qt::SquareCap,Qt::MiterJoin);
    painter->setPen(pen);
    QVector<QVector3D*> vecs;
    painter->translate(dotSize/2 ,dotSize/2); //reposition the dots

    //should'nt really be projected, but a fixed square.
    //just leaving it like this since we'll probably change the camera angle later on
    vecs.append(&glW->unProject(0,0));
    vecs.append(&glW->unProject(glW->width(),0));
    vecs.append(&glW->unProject(glW->width(),glW->height()));
    vecs.append(&glW->unProject(0,glW->height()));
    vecs.append(vecs.at(0));

    for (int i = 0;i <= 3;i++) {
        painter->drawLine(QPointF(vecs.at(i)->x(),-vecs.at(i)->y()),QPointF(vecs.at(i+1)->x(),-vecs.at(i+1)->y()));
    }

    painter->resetTransform();
    painter->setClipping(false);
}
