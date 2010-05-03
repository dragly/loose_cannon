#include "ui.h"
#include "glwidget.h"
#include "window.h"
#include "radar.h"
#include "cbutton.h"
#include "hudicon.h"


Ui::Ui(GLWidget* glW)
{
    this->glW=glW;
    mouseDown=false;

    //create standard hud objects.
    new Radar(this);


    Window* baseMenu;
    Cbutton* btn;
    baseMenu = new Window(this,0,0,0.2,0.2,Window::Center,true,"Units");
    btn = new Cbutton(baseMenu,QPointF(0.015,0.05),"Tank");
    HudIcon* hi = new HudIcon(baseMenu,Window::TopRight,0,0.5,"Units");
    GLWidget::connect(btn, SIGNAL(btnClicked()),glW, SLOT(recruitUnit()));
    GLWidget::connect(hi,SIGNAL(iconClicked()),baseMenu,SLOT(changeState()));
}

void Ui::addHudObject(HudObject* object) {
    hudObjects.prepend(object);
}

void Ui::resize() {
    for (int i=0; i<hudObjects.size(); i++) {
        hudObjects.at(i)->resize();
    }
}

void Ui::moveToFront(HudObject* object) {
    //NB: will crash if 'window' is not part of the list.
    hudObjects.move(hudObjects.indexOf(object),0);
    selectedHudObject=object;
}

void Ui::convertMousePos(int x, int y) {
    // scale off of the height
    mouseX = x/*/glW->height()*/;
    mouseY = y;
}

void Ui::draw(QPainter* painter) {

    for ( int i = hudObjects.size()-1;i>=0;i--) { // loop trough all windows in reverse order
        hudObjects.at(i)->draw(painter);
    }
}
bool Ui::isMouseDown() {
    return mouseDown;
}

//Ui::mouseMove etc..
void Ui::mouseRelease() {
    mouseDown=false;
    if (selectedHudObject != NULL/* && selectedHudObject->hovers()*/) {
        selectedHudObject->clickRelease();
    }
}

void Ui::move() {
    if (selectedHudObject != NULL) {
        selectedHudObject->move(mouseX-lastMouseX,mouseY-lastMouseY);
    }
    lastMouseX=mouseX;
    lastMouseY=mouseY;
    //selected hudobject.. instead of selected window Move();
}

bool Ui::mouseClick() {
    mouseDown=true;

    for (int i=0; i<hudObjects.size(); i++) {
        HudObject* object = hudObjects.at(i);
        if (object->click()) {
            hudObjects.move(i,0);
            selectedHudObject=object; //this one should always be at slot 0 to be drawn on top.
            lastMouseX=mouseX;
            lastMouseY=mouseY;
            return true;
        }
    }

    //if the last mouse press was in a menu but this one wasn't, hide all windows
    if (selectedHudObject != NULL) {
        for (int i=0; i<hudObjects.size(); i++) {
            hudObjects.at(i)->hide();
        }
        selectedHudObject=NULL;
    }

    return false;
}
