#include "window.h"
#include "glwidget.h"
#include "controller.h"

#include "ui.h"

const QColor Window::ColorNone = QColor(0,0,0,0);
const QColor Window::ColorBackground = QColor(25, 25, 25, 180);
const QColor Window::ColorBorder = QColor(127, 127, 127, 255);

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
    this->hidden = false; //temp

    ui->addWindow(this);

    //does currently NOT allow resizing of window after construction.
    this->size.setWidth(sizeX * ui->glW->height());
    this->size.setHeight(sizeY * ui->glW->height());

  if (!projected) {
        qreal xAlig, yAlig;
        if (alignment <= Window::TopRight)              //top
            yAlig = 0;
        else if  (alignment >= Window::BottomLeft)      //bottom
            yAlig =  ui->glW->height() - size.height();
        else                                            //middle
            yAlig =  ui->glW->height() / 2 - size.height()/2;

        if (alignment % 3 == 0)                         //left
            xAlig=0;
        else if (alignment % 3 == 1)                    //center
            xAlig= this->ui->glW->width()/2 - size.width()/2;
        else                                            //right
            xAlig=ui->glW->width() - size.width();

        this->pos.setX(x * ui->glW->height() + xAlig);
        this->pos.setY(y * ui->glW->height() + yAlig);
    } else {
        this->pos.setX(x * ui->glW->height());
        this->pos.setY(y * ui->glW->height());
    }
}

void  Window::addController(Controller* controller) {
    controllers.prepend(controller);
}

void Window::hide() {
   // hidden=true;
}

void Window::draw(QPainter* painter) {

    if (hidden)
        return;
    //draw the window itself
    drawBackground(painter);

    //draw the elements in the window
    for ( int i = controllers.size()-1;i>=0;i--) { // loop trough all windows in reverse order
        controllers.at(i)->draw(painter);

    }

}

bool Window::click() {

    //check if we hover at all
    if (ui->mouseX < pos.x() || ui->mouseX > pos.x()+size.height() || ui->mouseY < pos.y() || ui->mouseY > pos.y()+size.width())
        return false;

    //check if we pressed the titlebar
    if (!projected && /*hovering the titlebar*/ false) {
        return true;
    }
    //check all elements
    for (int i=0; i<controllers.size(); i++) {
        Controller* control = controllers.at(i);
        if (control->click()) {
            selectedController=control; //this one should always be at slot 0, to be drawn on top.
            return true;
        }
    }

    return true;
}



void Window::drawBackground(QPainter* painter) {
    QPoint location = pos;

    if (projected) {
        location += ui->glW->project(*world);
    }
    //border color
    //FillColor
    //Text Color
    //titlebar width == radius
    qreal radius = 0.025 * ui->glW->height();
    QPen pen(Window::ColorBorder,ui->glW->height()*0.0025,Qt::SolidLine,Qt::SquareCap,Qt::MiterJoin);


    //draw the titlebar as just another layer of the background ..for now
    if (titlebar) {
        painter->setPen(QColor(0,0,0,0));
        painter->setBrush(Window::ColorBackground);

        painter->drawPie(location.x(),location.y(),radius*2,radius*2,16*90,16*90);
        painter->drawPie(location.x() + size.width()-radius*2,location.y(),radius*2,radius*2,0,16*90);
        painter->drawRect(location.x() + radius, location.y(), size.width()- radius*2 , radius);
        painter->drawRect(location.x(),location.y() + radius,size.width(),radius/2);
    }

    //draw the window itself
    painter->setPen(pen);
    painter->setBrush(Window::ColorBackground);
    painter->drawRoundedRect(QRect(location,size),radius,radius);

    //draw the title and titlebar line
    if (titlebar) {
        QFont font;
        font.setPixelSize(radius);
        painter->setFont(font);
        painter->drawLine(location.x(),location.y()+radius*1.5,location.x()+size.width(),location.y()+radius*1.5);
        painter->drawText(QRect(location.x(),location.y(),size.width(),radius*1.5),Qt::AlignHCenter | Qt::AlignBottom, title);
    }
}
