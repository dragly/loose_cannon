#include "model.h"
#include <QtOpenGL>
#include <QPainter>
#include "glwidget.h"
#include "ui/window.h"
#include "entity.h"

Entity::Entity() {
    initEntity();
    type = TypeUnit;
}

Entity::Entity(Model *model) {
    initEntity();
    type = TypeUnit;
    this->model = model;
}

Entity::Entity(Model *model, int type) {
    initEntity();
    this->type = type;
    this->moveTarget = NULL;
    this->model = model;
}
Entity::~Entity() {
    if (this->menu != NULL)
        delete menu;
}

void Entity::initEntity() {
    this->menu = NULL;
    scale = QVector3D(1,1,1);
    velocity = QVector3D(0,0,0);
    rotation = QVector3D(0,0,0);
    health = GLWidget::MaxHealth;
    currentTarget = NULL;
    team = GLWidget::TeamHumans;
    moveState = StateStopped;
    movingAwayFrom = NULL;
}


void Entity::setWaypoints(QList<Node*> waypoints) {
    this->waypoints = waypoints;
    if(waypoints.count() > 0) {
        this->moveTarget = waypoints.first();
        this->moveState = StateMoving;
    } else {
        this->moveState = StateStopped;
    }
}

void Entity::addMenuPoitner(Window* menu) {
    this->menu = menu;
}
void Entity::select() {
    if (this->menu != NULL)
        menu->show();
}
bool Entity::isMoving() {
    if(moveState == StateMoving || moveState == StateMovingOutOfTheWay) {
        return true;
    } else {
        return false;
    }
}

void Entity::setModel(Model *model) {
    this->model = model;
}

void Entity::transform(QMatrix4x4 *modelview, int transformation) {
    if(transformation & Translate) {
        modelview->translate(position);
    }
    if(transformation & Rotate) {
        modelview->rotate(rotation.z(), 0, 0, 1);
        modelview->rotate(rotation.y(), 0, 1, 0); // do x and y last
        modelview->rotate(rotation.x(), 1, 0, 0);
    }
    if(transformation & Scale) {
        modelview->scale(scale);
    }
}

void Entity::draw(QMatrix4x4 modelview) {
    transform(&modelview);
    if(model != NULL) {
        model->draw(modelview);
    }
}
