#include "cbutton.h"
#include <QString>
#include <QPainter>
#include "window.h"
#include "ui.h"
#include "glwidget.h"

Cbutton::Cbutton(Window* parent, QPointF pos, QString text) : Controller(parent,pos,QSizeF(0.1,0.03))
{
    this->text = text;
}

bool Cbutton::click() {
    if (parent->ui->mouseX < pos.x() || parent->ui->mouseX > pos.x()+size.height() || parent->ui->mouseY < pos.y() || parent->ui->mouseY > pos.y()+size.width())
        return false;

    //call our function
    emit btnClicked();

    return true;
}

void Cbutton::draw(QPainter *painter) {
    QPoint location = pos;

    if (parent->projected) {
        location += parent->ui->glW->project(*parent->world);
    }

    QPen pen(Window::ColorBorder,parent->ui->glW->height()*0.0025,Qt::SolidLine,Qt::SquareCap,Qt::MiterJoin);

    painter->setPen(pen);
    painter->setBrush(Window::ColorBackground);
    painter->drawRect(QRect(location,size));

    QFont font;
    font.setPixelSize(0.02 * parent->ui->glW->height());
    painter->setFont(font);
    painter->drawText(QRect(location,size), Qt::AlignCenter, text);

}
void Cbutton::move() {
    //nada
}
