#include "window.h"
#include "glwidget.h"

#include "ui.h"

Window::Window(Ui* ui, qreal x,qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool projected, QVector3D* world, bool titlebar, QString title) {
    init(ui,x,y,sizeX,sizeY,alignment,projected,world,titlebar,title);
}

Window::Window(Ui* ui, qreal x,qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool titlebar, QString title) {
    init(ui,x,y,sizeX,sizeY,alignment,false,NULL,titlebar,title);
}

void Window::init(Ui* ui, qreal x,qreal y, qreal sizeX, qreal sizeY, Alignments alignment, bool projected, QVector3D* world, bool titlebar, QString title) {
    this->ui=ui;
    this->world = world;
    this->titlebar = titlebar;
    this->title = title;
    this->projected=projected;

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

void Window::draw(QPainter* painter) {

    //check if the window is hidden, blablabla

    //draw the window itself
    drawBackground(painter);

    //draw the elements in the window

}

void Window::Click() {

}

bool Window::Hovers() {
    return false;
}

void Window::drawBackground(QPainter* painter) {


    qreal radius = 0.05 * ui->glW->height();

    QRadialGradient gradient(50, 50, 50, 50, 50);
    gradient.setColorAt(0, QColor::fromRgbF(0, 1, 0, 1));
    gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));

    QBrush brush(gradient);
    painter->setBackground(brush);

    QRect(pos,size);
    painter->setPen(Qt::blue);
    painter->setBackgroundMode(Qt::OpaqueMode);
    painter->drawRoundedRect(QRect(pos,size),radius,radius);
    //painter.drawRects();
    //painter.drawRoundRect();
//    QString framesPerSecond;
//    framesPerSecond.setNum(frames /(frametime.elapsed() / 1000.0), 'f', 2);
//    painter.drawText(20, 40, framesPerSecond + " Bhoooooya!");
}

void Window::drawElements() {

}
