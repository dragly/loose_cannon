#include "ui.h"
#include "glwidget.h"
#include "window.h"


Ui::Ui(GLWidget* glW)
{
    this->glW=glW;
}

void Ui::addWindow(Window* window) {
    windows.prepend(window);
}

void Ui::convertMousePos(int x, int y) {
    // scale off of the height
    mouseX =   (qreal) x/*/glW->height()*/;
    mouseY = glW->height() - y;
}

void Ui::draw(QPainter* painter) {

    for ( int i = windows.size()-1;i>=0;i--) { // loop trough all windows in reverse order
        windows.at(i)->draw(painter);
    }
}

//Ui::mouseMove etc..
bool Ui::mouseClick() {

    for (int i=0; i<windows.size(); i++) {
        Window* window = windows.at(i);
        if (window->click()) {
            windows.move(i,0);
            selectedWindow=window; //this one should always be at slot 0, to be drawn on top.
            return true;
        }
    }

    selectedWindow=NULL;

    //for now just hide all windows?
    for (int i=0; i<windows.size(); i++) {
        Window* window = windows.at(i);
        window->hidden = true;
    }
    return false;

}
