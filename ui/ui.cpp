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

    if (clickMap())
        return true;

    return false;

}




bool Ui::clickMap () {

    const qreal Sqrt2 = 1.414213562373095;
    const qreal RadarSize=0.25; //size of the UiMap
    const qreal MAPSIZE = GLWidget::MapSize * GLWidget::NodeSize;
    const qreal MapWidth= MAPSIZE*2;
    const qreal MapHeight= MAPSIZE*2;

    qreal mapSize= RadarSize*glW->height()*2.55; //i got no idea where the factor 2.55 comes from :S

    //get the location of the coordinate system
    qreal mapCenterX =((qreal) glW->width() - ((qreal) glW->height()*RadarSize) /Sqrt2)/*-0.025*height*/;
    qreal mapCenterY = ((qreal) glW->height()*RadarSize)/Sqrt2;

    //translate
    qreal screenCoordX = mouseX - mapCenterX;
    qreal screenCoordY = mouseY - mapCenterY;

    //rotate
    qreal worldX = screenCoordX/Sqrt2 + screenCoordY/Sqrt2;
    qreal worldY = screenCoordX/Sqrt2 - screenCoordY/Sqrt2;

    //scale
    worldX*= mapSize/MapWidth;
    worldY*= mapSize/MapHeight;

    //clip
    if (worldX<-MAPSIZE || worldX > MAPSIZE || worldY <-MAPSIZE or worldY > MAPSIZE)
        return false;

    //move the view
    glW->offset.setX(worldX);
    glW->offset.setY(worldY);

    return true;
}

void Ui::drawMap(QPainter* painter) {

    //these should be fetched form glW
    const qreal MAPSIZE = GLWidget::MapSize * GLWidget::NodeSize; //copied from glwidgets mapsize, which (confusingly) should be multiplied with 2*nodedistance to get the actual map size.
    const qreal MapX = -MAPSIZE;
    const qreal MapY = -MAPSIZE;
    const qreal MapWidth= MAPSIZE*2;
    const qreal MapHeight= MAPSIZE*2;

    //temp vars
    const qreal Sqrt2 = 1.414213562373095;
    const qreal RadarSize=0.25; //size of the UiMap
    const qreal XPos = 0.05; //from the edge
    const qreal YPos = 0.05;
    const qreal DotSize = GLWidget::NodeSize;

    //if (dont draw map)
        //return;

    int height = glW->height();

    QPoint mapPos((qreal) glW->width() - (qreal) glW->height()*RadarSize*Sqrt2/2/*-0.025*height*/,0/*-0.025*height*/);
    qreal mapSize= RadarSize*height;

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
    painter->translate(- DotSize/2 - MapX,-DotSize/2 - MapY); //reposition the dots

    painter->setPen(Window::ColorNone);

    //draw buildings
    painter->setBrush(Qt::darkGreen);
    foreach(Entity* ent, glW->buildings) {
       painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),DotSize,DotSize));
    }

   painter->setBrush(Window::ColorNone);
   painter->setPen(Qt::green);

    //draw friends
    //painter->setBrush(Qt::green);
    foreach(Entity* ent, glW->units) {
        painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),DotSize,DotSize));
        //draw selected units in another color?
    }

    //draw enemies
    painter->setPen(Qt::red);
    foreach(Entity* ent, glW->enemies) {
        painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),DotSize,DotSize));
    }

    //draw the camera square  
    painter->setPen(Qt::white); //QPen pen(Qt::white,0,Qt::SolidLine,Qt::SquareCap,Qt::MiterJoin);
    QVector<QVector3D*> vecs;
    painter->translate(DotSize/2 ,DotSize/2); //reposition the dots

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
