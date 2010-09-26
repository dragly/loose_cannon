#include "controller.h"
#include "window.h"
#include "ui.h"
#include "glwidget.h"

Controller::Controller(Window* parent, QPointF posRelative,QSizeF sizeRelative)
{
    this->parent = parent;

    parent->addController(this);

    this->posRelative = posRelative;
    this->sizeRelative = sizeRelative;
}
void Controller::resize() {
    size.setWidth(sizeRelative.width() * parent->ui->glW->height());
    size.setHeight(sizeRelative.height() * parent->ui->glW->height());
    pos.setX(parent->pos.x() + posRelative.x() * parent->ui->glW->height());
    pos.setY(parent->pos.y() + posRelative.y() * parent->ui->glW->height());
}

bool Controller::click() {
    return false;
}

bool Controller::hovers() {
    return false;
}

void Controller::move() {

}

void Controller::draw(QPainter *painter) {
    Q_UNUSED(painter);
}
