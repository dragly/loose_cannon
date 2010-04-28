#include "ui.h"
#include "glwidget.h"
#include "window.h"


Ui::Ui(GLWidget* glW)
{
    this->glW=glW;
    windows.prepend(new Window(this,0,0,0.2,0.3,Window::TopLeft,true,new QVector3D(1,1,1),true,"Menu"));

}

void Ui::convertMousePos(int x, int y) {
    // scale off of the height
    mouseX =   (qreal) x/glW->height();
    mouseY = - (qreal) y/glW->height() + 1;
}

void Ui::draw(QPainter* painter) {

    for ( int i = windows.size()-1;i>=0;i--) { // loop trough all windows in reverse order
        windows.at(i)->draw(painter);
    }
}

//Ui::mouseMove etc..
bool Ui::mouseClick() {
    if (selectedWindow != NULL) {
        selectedWindow->Click();
    }

    for (int i=0; i<windows.size(); i++) {
        Window* window = windows.at(i);
        if (window->Hovers()) {
            windows.move(i,0);
            selectedWindow=window; //this one should always be at slot 0, to be drawn on top.
            selectedWindow->Click();
            return true;
        }
    }

    selectedWindow=NULL;
    //Stop drawing windows, if not in menu.. or anything else..
    return false;

}

