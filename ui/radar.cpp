#include "radar.h"
#include "ui.h"
#include "window.h"
#include "hudicon.h"
#include "entity.h"

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

Radar::Radar(Ui* ui)
{
    this->ui = ui;
    HudIcon* hi2 =new HudIcon(this,Window::TopRight,0,0,"Radar");
    QObject::connect(hi2,SIGNAL(iconClicked()), this, SLOT(changeState()));
    ui->addHudObject(this);
    this->hidden = true;
}
void Radar::draw(QPainter* painter) {

   if (!hidden)
        drawMap(painter);
}

void Radar::changeState() {
    hidden = !hidden;
}

bool Radar::click() {
    return (!hidden && clickMap());
}

void Radar::clickRelease() {
    if (!hidden)
        clickMap();
}

void Radar::hide() { //we want to keep the radar shown until minimized
    //hidden = true;
}

void Radar::show() {
    hidden = false;

}

void Radar::move(int x, int y) {
    if (!hidden)
        clickMap();
}

bool Radar::hovers() {
    return (!hidden && hoversMap());
}

bool Radar::hoversMap() {
    if (ui->mouseX < ui->glW->width() - RadarSize*Sqrt2*ui->glW->height() || ui->mouseY > ui->glW->height()*RadarSize*Sqrt2)
        return false;

    //first check if the mouse pointer is at all in the area
    if (ui->mouseX< ui->glW->width() - RadarSize*Sqrt2*ui->glW->height() || ui->mouseY > ui->glW->height()*RadarSize*Sqrt2)
        return false;

    qreal halfRadar= RadarSize*ui->glW->height()/2;

    //get the location of the coordinate system
    qreal mapCenterX =((qreal) ui->glW->width() - ((qreal) ui->glW->height()*RadarSize) /Sqrt2)/*-0.025*height*/;
    qreal mapCenterY = ((qreal) ui->glW->height()*RadarSize)/Sqrt2;

    //translate
    qreal screenCoordX = ui->mouseX - mapCenterX;
    qreal screenCoordY = ui->mouseY - mapCenterY;

    //rotate
    qreal worldX = screenCoordX/Sqrt2 + screenCoordY/Sqrt2;
    qreal worldY = screenCoordX/Sqrt2 - screenCoordY/Sqrt2;

    if (worldX<-halfRadar || worldX > halfRadar || worldY <-halfRadar || worldY > halfRadar)
        return false;
    else
        return true;
}

void Radar::resize() {

}

bool Radar::clickMap () {
    if (ui->mouseX < ui->glW->width() - RadarSize*Sqrt2*ui->glW->height() || ui->mouseY > ui->glW->height()*RadarSize*Sqrt2)
        return false;

    qreal mapSize = RadarSize*ui->glW->height()*2.55; //i got no idea why i have to use the factor 2.55 :S

    //get the location of the coordinate system
    qreal mapCenterX =((qreal) ui->glW->width() - ((qreal) ui->glW->height()*RadarSize) /Sqrt2)/*-0.025*height*/;
    qreal mapCenterY = ((qreal) ui->glW->height()*RadarSize)/Sqrt2;

    //translate
    qreal screenCoordX = ui->mouseX - mapCenterX;
    qreal screenCoordY = ui->mouseY - mapCenterY;

    //rotate
    qreal worldX = screenCoordX/Sqrt2 + screenCoordY/Sqrt2;
    qreal worldY = screenCoordX/Sqrt2 - screenCoordY/Sqrt2;

    //scale
    worldX*= mapSize/MapWidth;
    worldY*= mapSize/MapHeight;

    //clip
    if (worldX<-MAPSIZE || worldX > MAPSIZE || worldY <-MAPSIZE || worldY > MAPSIZE)
        return false;

    //move the view
    ui->glW->offset.setX(worldX);
    ui->glW->offset.setY(worldY);

    return true;
}



void Radar::drawMap(QPainter* painter) {


    //if (dont draw map)
        //return;

    int height = ui->glW->height();

    QPoint mapPos((qreal) ui->glW->width() - (qreal) ui->glW->height()*RadarSize*Sqrt2/2/*-0.025*height*/,0/*-0.025*height*/);
    qreal mapSize= RadarSize*height;

    painter->translate(mapPos);
    painter->rotate(45);

    //draw all surroundings and the map picture
    QRectF map(0,0,mapSize,mapSize);

//    painter->setClipRect(map); //clip stuff outside the map (mainly the camerasquare)

    painter->setPen(Window::ColorNone);
    painter->setBrush(Window::ColorBackground);
    painter->drawRect(map);

    //convert to world coords
    painter->scale(mapSize/MapWidth, mapSize/MapHeight);
    painter->translate(- DotSize/2 - MapX,-DotSize/2 - MapY); //reposition the dots

    painter->setPen(Window::ColorNone);

    //draw buildings
    painter->setBrush(Qt::darkGreen);
    foreach(Entity* ent, ui->glW->buildings) {
       painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),DotSize,DotSize));
    }

   painter->setBrush(Window::ColorNone);
   painter->setPen(Qt::green);

    //draw friends
    //painter->setBrush(Qt::green);
    foreach(Entity* ent, ui->glW->units) {
        painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),DotSize,DotSize));
        //draw selected units in another color?
    }

    //draw enemies
    painter->setPen(Qt::red);
    foreach(Entity* ent, ui->glW->enemies) {
        painter->drawRect(QRectF(ent->position.x(),- ent->position.y(),DotSize,DotSize));
    }

    //draw the camera square
    painter->setPen(Qt::white); //QPen pen(Qt::white,0,Qt::SolidLine,Qt::SquareCap,Qt::MiterJoin);
    QVector<QVector3D*> vecs;
    painter->translate(DotSize/2 ,DotSize/2); //reposition the dots

    //should'nt really be projected, but a fixed square.
    //just leaving it like this since we'll probably change the camera angle later on
    vecs.append(&ui->glW->unProject(0,0));
    vecs.append(&ui->glW->unProject(ui->glW->width(),0));
    vecs.append(&ui->glW->unProject(ui->glW->width(),ui->glW->height()));
    vecs.append(&ui->glW->unProject(0,ui->glW->height()));
    vecs.append(vecs.at(0));

    for (int i = 0;i <= 3;i++) {
        painter->drawLine(QPointF(vecs.at(i)->x(),-vecs.at(i)->y()),QPointF(vecs.at(i+1)->x(),-vecs.at(i+1)->y()));
    }

    painter->resetTransform();
//    painter->setClipRect(0,0,ui->glW->width(),height);//setClipping(false) doesnt seem to have any effect :<
//    painter->setClipping(false);

}
