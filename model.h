//    Copyright (C) 2010 Svenn-Arne Dragly <s@dragly.com>
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//    Some parts of the code might still be from Nokia's Qt examples
//    and are of course Copyright (C) Nokia and/or its subsidiary(-ies).

#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QtOpenGL>
#include <QPainter>
#include <QPaintEngine>

#include "glm.h"

class ModelTriangle
{
public:
    QVector3D vertices[3];
    QVector3D normals[3];
    QVector3D texcoords[3];
signals:

public slots:

};

class ModelGroup
{
public:
    QVector<ModelTriangle*> triangles;
signals:

public slots:

};


class Model
{
public:
    Model(QString filename);

    void load(QString filename);
    void draw(QMatrix4x4 modelview);
    void setTexture(GLuint texture);
    bool setShaderFiles(QString fragmentShader, QString vertexShader);
    bool setVertexShaderFile(QString filename);
    bool setFragmentShaderFile(QString filename);
    bool linkShaderProgram();
    bool initShaderProgram();
    void setShaderProgram(QGLShaderProgram *program);
    QVector3D scale;
private:
    // variables
    GLMmodel *model;
    GLuint texture;
    QGLShaderProgram *program;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<ModelGroup> groups;
    int vertexAttr;
    int normalAttr;
    int matrixUniform;
    int texCoordAttr;
    int textureUniform;
};

class Window;

class Entity
{
public:
    enum Type {TypeUnit, TypeBuilding, TypeBullet};
    enum MoveState {StateMoving, StateStopped, StateMovingOutOfTheWay, StateQueued}; // Moving = going to waypoint, Queued = waiting for waipoint to become free.

    Entity(Model *model);
    Entity(Model *model, int type);
    ~Entity();

    void addMenuPoitner(Window* menu);
    void select();
    void initEntity(Model *model);
    // functions

    void draw(QMatrix4x4 modelview);
//    void setTexture(GLuint texture);
//    bool setShaderFiles(QString fragmentShader, QString vertexShader);
//    bool setVertexShaderFile(QString filename);
//    bool setFragmentShaderFile(QString filename);
//    bool linkShaderProgram();
//    bool initShaderProgram();
    void setModel(Model *model);
    void setWaypoints(QList<Entity*> waypoints);
    bool isMoving();
    QList<Entity*> waypoints;
    Entity* moveTarget;

    // variables
    QVector3D position;
    QVector3D velocity;
    QVector3D rotation;
    QVector3D scale;

    qreal health;
    Entity* currentTarget;
    bool useMoveTarget; // shall we move to a target? Often set to false after currentTarget has been set.
    qreal lastBulletFired;
    Entity* positionNode;


    int team;
    int type;
    int moveState;

    Entity* movingAwayFrom;

private:
    Model *model;
    Window* menu; //export if we make building a child entity class
};

#endif // MODEL_H
