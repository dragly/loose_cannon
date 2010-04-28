#include "controller.h"
#include "window.h"
#include "ui.h"
#include "glwidget.h"

Controller::Controller(Window* parent, QPointF relativePos,QSizeF size)
{
    this->parent = parent;

    parent->addController(this);

    this->pos.setX(parent->pos.x() + relativePos.x() * parent->ui->glW->height());
    this->pos.setY(parent->pos.y() + relativePos.y() * parent->ui->glW->height());

    this->size.setWidth(size.width() * parent->ui->glW->height());
    this->size.setHeight(size.height() * parent->ui->glW->height());
}

bool Controller::click() {
    return false;
}

void Controller::move() {

}

void Controller::draw(QPainter *painter) {

}
