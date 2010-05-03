#include "ui.h"
#include "glwidget.h"
#include "window.h"
#include "radar.h"


Ui::Ui(GLWidget* glW)
{
    this->glW=glW;
    mouseDown=false;

    addHudObject(new Radar(this));
}

void Ui::addHudObject(HudObject* object) {
    hudObjects.prepend(object);
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
