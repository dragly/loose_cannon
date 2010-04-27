#include "window.h"
#include "glwidget.h"

#include "ui.h"

Window::Window(Ui* ui, qreal x,qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool projected, QVector3D* world, bool titlebar, QString title) {
    this->ui=ui;
    this->alignment = alignment;
    this->world = world;
    this->titlebar = titlebar;
    this->title = title;

    //does currently NOT allow resizing of window after construction.
    this->size.setWidth(sizeX * ui->glW->height());
    this->size.setHeight(sizeY * ui->glW->height());

    qreal xAlig, yAlig;
    if (alignment <= Window::TopRight)              //top
        yAlig = 0;
    else if  (alignment >= Window::BottomLeft)      //bottom
        yAlig =  ui->glW->height() - size.height();
    else                                            //middle
        yAlig =  ui->glW->height() / 2 - size.height()/2;

    if (alignment % 3 == 0)                         //left
        xAlig=0;
    else if (alignment % 3 == 1)                    //right
        xAlig=ui->glW->width() - size.width();
    else                                            //center
        xAlig= this->ui->glW->width()/2 - size.width()/2;


    this->pos.setX(x * ui->glW->height() + xAlig);
    this->pos.setY(y * ui->glW->height() + yAlig);
}

Window::Window(Ui* ui, qreal x,qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool titlebar, QString title) {
    Window(ui,x,y,sizeX,sizeY,alignment,false,NULL,titlebar,title);

}
void Window::Draw(QPainter* painter) {

    //check if the window is hidden, blablabla

    //draw the window itself
    drawBackground();

    //draw the elements in the window

}

void Window::Click() {

}

bool Window::Hovers() {
    return false;
}

void Window::drawBackground() {
    int i=2;
    //ui->convertMousePos(100,100);
    //ui->glW->height();
    //qreal radius =/* 0.05 */ (qreal) ui->glW->height(); //test
    //QPainter painter;
    //QRect(pos,size);
    //painter.begin(ui->glW);
   // painter.setPen(Qt::blue);
    //painter.drawRoundedRect(QRect(pos,size),radius,radius);

    //painter.drawRects();
    //painter.drawRoundRect();
//    QString framesPerSecond;
//    framesPerSecond.setNum(frames /(frametime.elapsed() / 1000.0), 'f', 2);
//    painter.drawText(20, 40, framesPerSecond + " Bhoooooya!");
}

void Window::drawElements() {

}
