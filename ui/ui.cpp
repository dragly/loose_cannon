#include "ui.h"
#include "glwidget.h"
#include "window.h"


Ui::Ui(GLWidget* glW)
{
    this->glW=glW;

    //temp

}

void Ui::initialize() {
    //calling it from the constructor fucked with my pointer :< still did :<
    windows.prepend(new Window(this,0,0,0.2,0.2,Window::Center));
}

void Ui::convertMousePos(int x, int y) {
    // scale off of the height
    mouseX =   x/glW->height();
    mouseY = - y/glW->height() + 1;
}

void Ui::Draw(QPainter* painter) {

    for ( int i = windows.size()-1;i>=0;i--) { // loop trough all windows in reverse order
        windows.at(i)->Draw(painter);

        if (this != windows.at(i)->ui) {
            painter->drawText(60, 100,"Allocated memory got moved");
        } else
            painter->drawText(60, 100,"Everything seems ok");

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

