#ifndef ENTITY_H
#define ENTITY_H

#include <QtOpenGL>

class Model;
class Window;
class Node;

class Entity : public QObject
{
    Q_OBJECT
public:
    enum Type {TypeUnit, TypeBuilding, TypeBullet};
    enum MoveState {StateMoving, StateStopped, StateMovingOutOfTheWay, StateQueued}; // Moving = going to waypoint, Queued = waiting for waipoint to become free.
    enum AttackState {OrderAttack, OrderMove}; // Moving = going to waypoint, Queued = waiting for waipoint to become free.
    enum Transformation {Rotate = 0x01, Translate = 0x02, Scale = 0x04};
    Entity();
    Entity(Model *model);
    Entity(Model *model, int type);
    ~Entity();

    void addMenuPoitner(Window* menu);
    void select();
    void initEntity();
    // functions

    void transform(QMatrix4x4 *modelview, int transformations = Rotate | Scale | Translate);
    virtual void draw(QMatrix4x4 modelview);
//    void setTexture(GLuint texture);
//    bool setShaderFiles(QString fragmentShader, QString vertexShader);
//    bool setVertexShaderFile(QString filename);
//    bool setFragmentShaderFile(QString filename);
//    bool linkShaderProgram();
//    bool initShaderProgram();
    void setModel(Model *model);
    void setWaypoints(QList<Node*> waypoints);
    bool isMoving();
    QList<Node*> waypoints;
    Node* moveTarget;

    // variables
    QVector3D position;
    QVector3D velocity;
    QVector3D rotation;
    QVector3D scale;

    qreal health;
    Entity* currentTarget;
    bool useMoveTarget; // shall we move to a target? Often set to false after currentTarget has been set.
    qreal lastBulletFired;
    Node* positionNode;

    int team;
    int type;
    int moveState;
    int orders;

    Entity* movingAwayFrom;
protected:
    Model *model;
    Window* menu; //export if we make building a child entity class
};

#endif // ENTITY_H
