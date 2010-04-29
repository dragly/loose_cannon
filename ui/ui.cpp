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
