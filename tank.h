#ifndef TANK_H
#define TANK_H

#include "unit.h"

class Model;

class Tank : public Unit
{
    Q_OBJECT
public:
    Tank(Model *bodyModel, Model *towerModel);
    void draw(QMatrix4x4 modelview);
    QVector3D towerRotation;
private:
    Model* bodyModel;
    Model* towerModel;
};

#endif // TANK_H
