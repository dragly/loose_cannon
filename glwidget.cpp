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

#include "glwidget.h"
#include "model.h"
#include "entity.h"
#include "tank.h"
#include "ui/ui.h"
#include <QPainter>
#include <QPaintEngine>
//#include <Phonon/MediaObject>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
//#include "ui/window.h"
//#include "ui/cbutton.h"
#include "node.h"
#include "soundthread.h"

// constants
const qreal UnitSpeed = 40.0; // m/s
const qreal UnitAcceleration = 10.0; // m/s^2
const qreal UnitFrictionSide = 6.0;
const qreal UnitFrictionAll = 2.0;
const qreal EnemySpawnDistance = GLWidget::NodeSize * 8; // m
const qreal RotateSpeed = 60; // degrees/s
const qreal TowerRotateSpeed = 120; // degrees/s
const qreal BulletSpeed = 40; // m/s
const qreal NumberOfEnemies = 1;

// map and nodes
const int GLWidget::MapSize; //Vj: moved to header as i need them in another class
const qreal GLWidget::NodeSize;
//const int MapSize = 30; // 2n x 2n nodes
//const qreal NodeSize = 8; // each node is 8x8 m (the length of a tank)
const qreal NodeSizeSquared = GLWidget::NodeSize * GLWidget::NodeSize; // just for convenience
const qreal NodeSizeDiagonal = GLWidget::NodeSize * GLWidget::NodeSize + GLWidget::NodeSize * GLWidget::NodeSize; // just for convenience
const qreal UnitCollideDistance = NodeSizeSquared / 2.5; // divided by two because we shouldn't be too nazi ;) now units may move diagonally without interrupting neighbor nodes.

// gui and interactions
const qreal DragDropTreshold = 20;
const qreal ClickRadius = GLWidget::NodeSize / 2.0;

// weapon constants
const qreal ExplosionRadiusSquared = 3*3; // squared
const qreal ExplosionDamage = 30;
const qreal ExplosionForce = 20;
const qreal FireDistanceSquared = NodeSizeSquared * 6.0 * 6.0;
const qreal BulletSpawnTime = 2;
QVector3D Gravity(0, 0, -10); // m/s^2
const qreal GLWidget::MaxHealth;
GLWidget::~GLWidget()
{
}

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    //    soundbank = new KGrSoundBank(2);
    //    expsound = soundbank->loadSound("data/sounds/bomb-02.wav");
    //    expsound2 = soundbank->loadSound("data/sounds/bomb-02.ogg");
    //    qDebug() << QSound::isAvailable();
    //    explosion = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("data/sounds/explosion-02.ogg"));
    //    explosion2 = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("data/sounds/bomb-02.ogg"));
    //    explosion3 = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("data/sounds/bomb-03.ogg"));
    useSound = true;
    drawHud = true;

    qsrand(time(NULL));
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);
    monkeyModel = new Model("data/objects/monkey1.obj");
    boxModel = new Model("data/objects/box.obj");
    cannonModel = new Model("data/objects/cannon.obj");
    humanTankBodyModel = new Model("data/objects/tank-body.obj");
    humanTankBodyModel->scale *= 0.5;
    humanTankTowerModel = new Model("data/objects/tank-head.obj");
    humanTankTowerModel->scale *= 0.5;
    enemyTankBodyModel = new Model("data/objects/tank-body.obj");
    enemyTankBodyModel->scale *= 0.5;
    enemyTankTowerModel = new Model("data/objects/tank-head.obj");
    enemyTankTowerModel->scale *= 0.5;
    bulletModel = new Model("data/objects/bullet.obj");
    bulletModel->scale *= 0.6;
    nodeModel = new Model("data/objects/box.obj");
    // initial values
    camera = QVector3D(25, -25, 80);
    sndExplosion = "data/sounds/bomb.wav";
    QStringList audioSamples;
    audioSamples << sndExplosion;
    soundThread = new SoundThread(this, audioSamples);
    soundThread->start();
    //    sndExplosion = soundThread->loadSample("data/sounds/bomb.wav");
    qDebug() << "main thread is" << QThread::currentThreadId();
    grabKeyboard();
    grabMouse();
    // timer, should be set last, just in case
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->setInterval(1);
    timer->start();
}

void GLWidget::resetGame() {
    regenerateNodes();
    // init all to zero (also avoids memory failures)
    lastFrameTime = 0.0;
    gameOver = false;
    dragging = false;
    inUi = false;
    frames = 0;
    score = 0;
    gameOverTime = 0.0;
    offset *= 0;
    recruitqueue = 0;
    // end init all to zero
    gametime.start();
    bullets.clear();
    enemies.clear();
    Tank* cannon = new Tank(humanTankBodyModel, humanTankTowerModel);
    cannon->position = QVector3D(10,5,1);
    cannon->positionNode = closestNode(cannon->position);
    cannon->team = TeamHumans;
    selectedUnit = cannon;
    units.append(cannon);
    Tank* cannon2 = new Tank(humanTankBodyModel, humanTankTowerModel);
    cannon2->position = QVector3D(1,1,1);
    cannon2->positionNode = closestNode(cannon2->position);
    cannon2->team = TeamHumans;
    units.append(cannon2);
    Entity* building = new Entity(boxModel, Entity::TypeBuilding);
    building->position = QVector3D(-4,4,0);
    building->positionNode = closestNode(building->position);
    building->health = 1000;

    //building->addMenuPoitner(baseMenu);
    buildings.append(building);
    initEnemies();
    testUnit = new Entity(boxModel);
    testUnit->scale *= 0.1;
}

void GLWidget::initEnemies() {
    for(int i = 0; i < NumberOfEnemies; i++) {
        createEnemy();
    }
}

void GLWidget::resetEnemy(Entity* enemy) {
    qreal randomAngle = qrand() * 360; // set random position
    enemy->position = QVector3D(cos(randomAngle * M_PI / 180) * EnemySpawnDistance, sin(randomAngle * M_PI / 180) * EnemySpawnDistance, 0.0); // set random position
    enemy->positionNode = closestNode(enemy->position);
    qDebug() << "Position enemy:" << enemy->position;
    enemy->health = MaxHealth; // reset health
    if(buildings.count() > 0)
        enemy->currentTarget = buildings.first(); // attack any buildling
    else if(units.count() > 0)
        enemy->currentTarget = units.first(); // or attack any unit
    enemy->orders = Entity::OrderAttack;
}

void GLWidget::createEnemy() {
    qDebug() << "Creating enemy";
    Tank *enemy = new Tank(enemyTankBodyModel, enemyTankTowerModel);
    enemy->team = TeamEnemies;
    enemies.append(enemy);
    resetEnemy(enemy);
}

void GLWidget::initializeGL ()
{
    glClearColor(0.8f, 0.7f, 0.8f, 1.0f);
    // create and set shaders
    QGLShaderProgram *program = new QGLShaderProgram(this);
    program->addShaderFromSourceFile(QGLShader::Fragment, "data/shaders/fshader.glsl");
    program->addShaderFromSourceFile(QGLShader::Vertex, "data/shaders/vshader.glsl");
    program->link();
    monkeyModel->setShaderProgram(program);
    cannonModel->setShaderProgram(program);
    bulletModel->setShaderProgram(program);
    boxModel->setShaderProgram(program);
    nodeModel->setShaderProgram(program);
    humanTankBodyModel->setShaderProgram(program);
    humanTankTowerModel->setShaderProgram(program);
    enemyTankBodyModel->setShaderProgram(program);
    enemyTankTowerModel->setShaderProgram(program);
    //    if(!monkeyModel->setShaderFiles("fshader.glsl","vshader.glsl")) {
    //        qDebug() << "Failed to set shader files.";
    //    }
    //    if(!cannonModel->setShaderFiles("fshader.glsl","vshader.glsl")) {
    //        qDebug() << "Failed to set shader files.";
    //    }
    //    if(!bulletModel->setShaderFiles("fshader.glsl","vshader.glsl")) {
    //        qDebug() << "Failed to set shader files.";
    //    }
    //    if(!boxModel->setShaderFiles("fshader.glsl","vshader.glsl")) {
    //        qDebug() << "Failed to set shader files.";
    //    }
    // end shaders
    // create and set textures
    GLuint furTexture;
    glGenTextures(1, &furTexture);
    furTexture = bindTexture(QImage("data/textures/fur.resized.jpg"));
    GLuint metalTexture;
    glGenTextures(1, &metalTexture);
    metalTexture = bindTexture(QImage("data/textures/metal.small.jpg"));
    GLuint armyTexture;
    glGenTextures(1, &armyTexture);
    armyTexture = bindTexture(QImage("data/textures/army-texture.png"));
    GLuint yellowArmyTexture;
    glGenTextures(1, &yellowArmyTexture);
    yellowArmyTexture = bindTexture(QImage("data/textures/army-texture-yellow.png"));
    boxModel->setTexture(furTexture);
    monkeyModel->setTexture(furTexture);
    cannonModel->setTexture(metalTexture);
    bulletModel->setTexture(metalTexture);
    humanTankBodyModel->setTexture(armyTexture);
    humanTankTowerModel->setTexture(armyTexture);
    enemyTankBodyModel->setTexture(yellowArmyTexture);
    enemyTankTowerModel->setTexture(yellowArmyTexture);
    // end textures

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    this->ui = new Ui(this);
    //    Window* menu = new Window(ui,0,0,0.2,0.3,Window::Center,true,"Menu");
    //    new Cbutton(menu,QPointF(0.015,0.05),"New game");
    resetGame();

}

void GLWidget::paintGL()
{
    qreal currentFrameTime = gametime.elapsed() / 1000.0;
    qreal dt = currentFrameTime - lastFrameTime;
    // Let's do physics!
    bool collision = false;
    //    while(lastFrameTime <= currentFrameTime) { // let the physics catch up with the current time
    QList<Entity*> allUnits; // all units, including enemies and our own
    allUnits.append(enemies);
    allUnits.append(units);
    QList<Entity*> allDestructibles; // all destructible things - units and buildings
    allDestructibles.append(allUnits);
    allDestructibles.append(buildings);
    if(!gameOver) { // do logic

        // if an enemy sees a closer target, he will select it (this is to make it fire at units instead of buildings and should be deprecated TODO)
        foreach(Entity* enemy, enemies) {
            if(enemy->currentTarget != NULL) {
                qreal currentDistance = (enemy->currentTarget->position - enemy->position).lengthSquared();
                foreach(Entity* unit, units) {
                    qreal distance = (unit->position - enemy->position).lengthSquared();
                    if(currentDistance > distance) { // the enemy always selects the closest target
                        enemy->currentTarget = unit; // if we are closer, the enemy chooses to target us
                    }
                }
            } else {
                enemy->currentTarget = selectedUnit; // this should be the base, but we don't have a base yet
            }
        }

        // if there are enemies nearby and we don't have a target yet, let's shoot!
        foreach(Entity* aunit, allUnits) {
            if(aunit->currentTarget == NULL) {
                foreach(Entity* targetUnit, allUnits) {
                    if(targetUnit->team != aunit->team && (aunit->position - targetUnit->position).lengthSquared() < FireDistanceSquared) {
                        aunit->currentTarget = targetUnit;
                    }
                }
            }
        }

        foreach(Entity* bullet, bullets) { // let's see what our bullets are doing
            bool hitUnit = false;
            bullet->velocity += Gravity * dt;
            bullet->position += bullet->velocity * dt;
            qreal bulletAngleZ = atan2(bullet->velocity.y(),bullet->velocity.x()) * 180 / M_PI + 90;
            bullet->rotation.setZ(bulletAngleZ);
            qreal bulletAngleX = atan2(sqrt(pow(bullet->velocity.x(),2) + pow(bullet->velocity.y(),2)),
                                       bullet->velocity.z()) * 180 / M_PI + 90;
            bullet->rotation.setX(bulletAngleX);
            foreach(Entity* aunit, allUnits) {
                QVector3D distance = bullet->position - aunit->position;
                if(distance.lengthSquared() < 3 && bullet->team != aunit->team) {
                    hitUnit = true;
                }
            } // foreach enemy
            if(bullet->position.z() < 0 || hitUnit) { // we have an explosion
                // TODO: Animate explosion with sprites as seen here: http://news.developer.nvidia.com/2007/01/tips_strategies.html
                if(useSound) {
                    emit playSound(sndExplosion); // play the sound in the soundBank thread
                }
                foreach(Entity *hitUnit, allDestructibles) {
                    QVector3D distance = hitUnit->position - bullet->position;
                    if(distance.lengthSquared() < ExplosionRadiusSquared) { // in explosion radius
                        qreal damage = ExplosionDamage * (distance.lengthSquared() / ExplosionRadiusSquared); // the damage is relative to the distance
                        hitUnit->health -= damage;
                        score += damage;
                        if(hitUnit->currentTarget == NULL && // if we have not selected a target
                           hitUnit->isMoving() == false && // and we are not moving anywhere
                           hitUnit->team != bulletOwner[bullet]->team && // and the guy shooting on us is not on our team
                           bulletOwner[bullet]->health > 0) { // and he's not dead
                            hitUnit->currentTarget = bulletOwner[bullet]; // then get back at that bastard!
                        }
                        if(hitUnit->health < 0) { // we're dead
                            enemies.removeAll(hitUnit); // remove us from whatever list we came from
                            units.removeAll(hitUnit);
                            if(hitUnit->team == TeamEnemies) { // if it was an enemy, create a new one
                                createEnemy();
                            }
                            foreach(Entity* aunit, allUnits) { // if someone had us as a target, we're no target no more :(
                                if(aunit->currentTarget == hitUnit) {
                                    aunit->currentTarget = NULL;
                                    if(hitUnit == selectedUnit) {
                                        if(units.count() > 0)
                                            selectedUnit = units.first();
                                    }
                                }
                            }
                        } /*else if(hitUnit->type != Entity::TypeBuilding){
                                qreal velocityChange = ExplosionForce * damage / 100;
                                hitUnit->velocity += distance.normalized() * velocityChange; // make the explosion change the velocity in the direction of the blast
                            }*/
                    }
                }
                bullets.removeOne(bullet);
            } // endif hit
        } // end foreach bullets
        // start collision detection
        QList<Entity*> alreadyChecked;
        QList<Entity*> collisions;
        foreach(Entity* aunit, allUnits) { // foreach all units
            // collision detection (path finding)
            bool foundCollision = false;
            foreach(Entity* collideUnit, allUnits) {
                if(collideUnit == aunit)
                    continue; // if it is ourselves, we of course don't collide and we don't need to check both A & B and B & A.
                if(alreadyChecked.contains(collideUnit))
                    continue;
                if((collideUnit->position - aunit->position).lengthSquared() < UnitCollideDistance) {
                    collisions.append(collideUnit);
                    foundCollision = true;
                    collision = true;
                    if(aunit->team == collideUnit->team) { // we don't give orders to the other team's units
                        if(aunit->moveState == Entity::StateStopped && collideUnit->moveState == Entity::StateStopped) { // worst case scenario - they shouldn't come this far
                            if(nodeNeighbors[aunit->positionNode].count() > 0) {
                                //                                    qDebug() << "collision findpath";
                                int randomInt = (int)((qreal) nodeNeighbors[aunit->positionNode].count() * (qreal) qrand() / (qreal) RAND_MAX);
                                aunit->setWaypoints(findPath(aunit->positionNode, nodeNeighbors[aunit->positionNode].at(randomInt))); // should probably do a random selection
                                aunit->moveState = Entity::StateMovingOutOfTheWay;
                                aunit->movingAwayFrom = collideUnit;
                                //                                    qDebug() << "collision states" << aunit->moveState << collideUnit->moveState;
                            } else {
                                //                                    qDebug() << "collision statestopped, but could not find neighbors";
                            }
                        } else if(aunit->moveState == Entity::StateMoving || collideUnit->moveState == Entity::StateMoving || aunit->moveState == Entity::StateMovingOutOfTheWay || collideUnit->moveState == Entity::StateMovingOutOfTheWay) { // only one is moving
                            if((aunit->moveState == Entity::StateMovingOutOfTheWay && aunit->movingAwayFrom == collideUnit)
                                || (collideUnit->moveState == Entity::StateMovingOutOfTheWay && collideUnit->movingAwayFrom == aunit)) {
                                continue; // if we are moving away from the other unit, let's not make new plans because of a new collision
                            }
                            //                                qDebug() << "collision moving states" << aunit->moveState << collideUnit->moveState;
                            Entity *movingUnit; // the moving unit
                            Entity *stoppedUnit; // the unit standing still
                            if(aunit->moveState == Entity::StateMoving) {
                                movingUnit = aunit;
                                stoppedUnit = collideUnit;
                            } else {
                                movingUnit = collideUnit;
                                stoppedUnit = aunit;
                            }
                            if(stoppedUnit->moveState != Entity::StateQueued &&
                               movingUnit->moveState != Entity::StateQueued &&
                               stoppedUnit->moveState != Entity::StateMovingOutOfTheWay &&
                               movingUnit->moveState != Entity::StateMovingOutOfTheWay) {
                                //                                    qDebug() << "finding nodes";
                                QList<Node*> avoidNodes;
                                avoidNodes.append(movingUnit->waypoints);
                                avoidNodes.append(movingUnit->positionNode);
                                if(nodeNeighbors[aunit->positionNode].count() > 0) {
                                    QList<Node*> freeNodes;
                                    foreach(Node* node, nodeNeighbors[stoppedUnit->positionNode]) {
                                        if(!avoidNodes.contains(node)) { // make sure the unit asking to get past is not going to any of these waypoints
                                            freeNodes.append(node);
                                        }
                                    }
                                    if(freeNodes.count() > 0) {
                                        // let's select a random node - this keeps unit's away from all moving in the same direction
                                        int randomInt = (int)((qreal) freeNodes.count() * (qreal) qrand() / (qreal) RAND_MAX);
                                        Node* node = freeNodes.at(randomInt);
                                        //                                            qDebug() << "newWaypoints findpath";
                                        QList<Node*> newWaypoints;
                                        newWaypoints.append(findPath(stoppedUnit->positionNode, node, avoidNodes));
                                        if(stoppedUnit->waypoints.count() > 0) // if the one which is stopped was going somewhere, add the rest to the path
                                            newWaypoints.append(findPath(node, stoppedUnit->waypoints.last()));
                                        stoppedUnit->setWaypoints(newWaypoints);
                                        stoppedUnit->movingAwayFrom = movingUnit; // we need to report back to this unit when we are done moving away
                                        movingUnit->moveState = Entity::StateQueued; // set states
                                        stoppedUnit->moveState = Entity::StateMovingOutOfTheWay;
                                    } else { // could not find free node
                                        movingUnit->moveState = Entity::StateStopped;
                                        stoppedUnit->moveState = Entity::StateStopped;
                                        //                                            qDebug() << "Could not find free node!";
                                    }
                                } else {
                                    //                                        qDebug() << "collision moving had no node neighbors!";
                                } // end if node neighbor count
                            } else {
                                //                                    qDebug() << "collision moving - was queued or moving out of the way";
                            }
                        } /*else if(aunit->moveState == Entity::StateQueued && collideUnit->moveState == Entity::StateQueued){
                                aunit->moveState = Entity::StateMovingOutOfTheWay; // just to avoid getting stuck
                            }*/
                        // end if state
                    } else {
                        // just stop both units if they are enemies
                        //                            aunit->moveState = Entity::StateStopped;
                        //                            collideUnit->moveState = Entity::StateStopped;
                    } // end if same team
                } // end if collided
            } // end foreach all units
            alreadyChecked.append(aunit);
            if(!foundCollision && !collisions.contains(aunit)) { // if we no longer collide, stop ourselves from waiting
                if(aunit->moveState == Entity::StateMovingOutOfTheWay || aunit->moveState == Entity::StateQueued) {
                    aunit->moveState = Entity::StateMoving;
                    qDebug() << "Cleared collision";
                }
            }
            // end collision detection
            // set the unit to attack its target (rotate/direction)
            QVector3D aunitdir;
            bool shallMove = false;
            if(aunit->isMoving() && aunit->moveTarget != NULL) {
                aunitdir = aunit->moveTarget->position - aunit->position;
                shallMove = true;
            }
            if(aunit->currentTarget != NULL && aunit->orders == Entity::OrderAttack) { // if we have an enemy to kill and we're ordered to attack
                if((aunit->currentTarget->position - aunit->position).lengthSquared() < FireDistanceSquared) {
                    aunit->waypoints.clear();
                } else {
                    if(!aunit->isMoving()) {
                        qDebug() << "not moving but too far away";
                        aunit->setWaypoints(findPath(aunit->positionNode, aunit->currentTarget->positionNode));
                    }
                }
            }
            qreal goalAngle = atan2(aunitdir.y(),aunitdir.x()) * 180 / M_PI + 90;
            qreal goalAngleDifference = goalAngle - aunit->rotation.z();
            // end direction calculations
            if(shallMove) {
                bool doMovement = true;
                while(goalAngleDifference > 180) goalAngleDifference -= 360;
                while(goalAngleDifference < -180) goalAngleDifference += 360;
                if(goalAngleDifference > 0) {
                    aunit->rotation.setZ(aunit->rotation.z() + RotateSpeed * dt);
                    if(goalAngleDifference - RotateSpeed * dt < 0) {
                        aunit->rotation.setZ(goalAngle);
                        doMovement = true;
                    }
                } else if(goalAngleDifference < 0) {
                    aunit->rotation.setZ(aunit->rotation.z() - RotateSpeed * dt);
                    if(goalAngleDifference + RotateSpeed * dt > 0) {
                        aunit->rotation.setZ(goalAngle);
                        doMovement = true;
                    }
                }

                if(aunit->moveTarget != NULL) {
                    if((aunit->position - aunit->moveTarget->position).lengthSquared() < NodeSizeSquared / 2) {
                        aunit->positionNode = aunit->moveTarget;
                        if(aunit->waypoints.count() > 0) { // we still got somewhere to go
                            aunit->moveTarget = aunit->waypoints.first();
                            aunit->waypoints.removeFirst();
                            qDebug() << "Going to:" << aunit->moveTarget->position;
                            doMovement = true;
                        } else { // we are too close and we have no more places to go, let's get a bit closer, then stop
                            if(aunit->movingAwayFrom != NULL) { // If we were moving away from someone, report back to them that they should move, and that we are not moving away from them anymore
                                aunit->movingAwayFrom->moveState = Entity::StateMoving;
                                aunit->movingAwayFrom = NULL;
                                qDebug() << "Reporting back to friend";
                            }
                            if((aunit->position - aunit->moveTarget->position).lengthSquared() < 1) { // move to the center of the node
                                aunit->moveState = Entity::StateStopped; // we are no longer to move towards a target since we are already there!
                                aunit->moveTarget = NULL;
                                doMovement = false;
                                qDebug() << "At target!";
                            } else {
                                doMovement = true;
                            }
                        }
                    }
                }
                if(aunit->moveState == Entity::StateStopped) {
                    doMovement = false;
                }
                QVector3D vectorAcceleration;
                //                    vectorAcceleration += - aunit->velocity * UnitAcceleration / UnitSpeed; // air resistance acts in the negative direction of the velocity
                if(doMovement) { // only if we are moving, we will be affected by engine acceleration
                    qreal aunitAcceleration = UnitAcceleration;
                    vectorAcceleration += aunitdir.normalized() * aunitAcceleration;
                    //                        aunit->moveState = Entity::StateMoving;
                } // end doMovement
                // side-friction
                //                    if(aunit->position.z() == 0) {
                //                        QVector3D projected = aunit->velocity - QVector3D::dotProduct(aunit->velocity, aunitdir.normalized()) * aunitdir.normalized();
                //                        qreal aunitFriction = - UnitFrictionSide * projected.length(); // we find the amount of friction applied
                //                        aunit->velocity += projected.normalized() * aunitFriction * dt;	// apply the friction
                //                    }
                // all-way friction
                vectorAcceleration += - aunit->velocity * UnitFrictionAll;
                aunit->velocity += vectorAcceleration * dt;
            } else { // else shallMove
                aunit->velocity = QVector3D(0,0,aunit->velocity.z());
            } // end shallMove
            // Gravity
            if(aunit->position.z() > 0) {
                aunit->velocity += Gravity * dt;
            } else {
                aunit->velocity.setZ(0);
                aunit->position.setZ(0);
            }
            // rotate tower
            QVector3D bulletPosition;
            QVector3D direction;
            QVector3D calcTarget;
            QVector3D calcDirection;
            QVector3D towerDirection;
            qreal calcRotation;
            bool alreadySetRotation = false;
            if(aunit->currentTarget != NULL) {
                bulletPosition = aunit->position + QVector3D(0,0,0.4);
                direction = aunit->currentTarget->position - bulletPosition;
                calcTarget = aunit->currentTarget->position + aunit->currentTarget->velocity * direction.length() / BulletSpeed; // hit a bit ahead of target, suggesting same speed all the way
                calcDirection = calcTarget - bulletPosition;
                towerDirection = calcDirection;
            } else if(aunit->waypoints.count() > 0) {
                towerDirection = aunit->waypoints.last()->position - aunit->position;
            } else if(aunit->moveTarget != NULL) {
                towerDirection = aunit->moveTarget->position - aunit->position;
            } else {
                towerDirection = aunit->position;
                calcRotation = aunit->rotation.z();
                alreadySetRotation = true;
            }
            // rotate tower
            Tank* tankPointer = qobject_cast<Tank *>(aunit);
            qreal towerRotationDifference = 0;
            if(tankPointer != NULL) {
                if(!alreadySetRotation) {
                    calcRotation = atan2(towerDirection.y(), towerDirection.x()) * 180 / M_PI + 90;
                }
                towerRotationDifference = calcRotation - tankPointer->towerRotation.z();
                while(towerRotationDifference > 180) towerRotationDifference -= 360;
                while(towerRotationDifference < -180) towerRotationDifference += 360;
                if(towerRotationDifference > 0) {
                    tankPointer->towerRotation.setZ(tankPointer->towerRotation.z() + TowerRotateSpeed * dt);
                    if(towerRotationDifference - RotateSpeed * dt < 0) {
                        tankPointer->towerRotation.setZ(calcRotation);
                    }
                } else if(towerRotationDifference < 0) {
                    tankPointer->towerRotation.setZ(tankPointer->towerRotation.z() - TowerRotateSpeed * dt);
                    if(towerRotationDifference + RotateSpeed * dt > 0) {
                        tankPointer->towerRotation.setZ(calcRotation);
                    }
                }
            }
            // fire bullets
            if(aunit->currentTarget != NULL /*&& difference < 1 && difference > -1*/ && lastFrameTime - aunit->lastBulletFired > BulletSpawnTime) {
                if((aunit->currentTarget->position - aunit->position).lengthSquared() < FireDistanceSquared
                   && qAbs(towerRotationDifference) < 1) { // make sure we are close enough
                    Entity *bullet = new Entity(bulletModel, Entity::TypeBullet);
                    bullet->position = bulletPosition;
                    bullet->velocity = calcDirection.normalized() * BulletSpeed;
                    qreal bulletTime = calcDirection.length() / BulletSpeed;
                    qreal startSpeed = -Gravity.z() * bulletTime; // from v = v0 + at
                    bullet->velocity += QVector3D(0, 0, startSpeed * 0.5);
                    aunit->lastBulletFired = lastFrameTime;
                    bullet->team = aunit->team;
                    bullet->type = Entity::TypeBullet;
                    bulletOwner.insert(bullet,aunit);
                    bullets.append(bullet);
                }
            }
            aunit->position += aunit->velocity * dt; // do movement
        } // end foreach allUnits

        //recruitment of units //foreach all buildings and make the properties private or allocate them.
        if (recruitqueue > 0 && recruittime.elapsed() > 1000) {
            recruitqueue--;
            recruittime.restart();
            Tank* unit = new Tank(humanTankBodyModel, humanTankTowerModel);
            unit->position =/* aunit->position +*/ QVector3D(-4,6,6); //note, positioning might cause trouble with buildings at the edge of the map..
            unit->positionNode = closestNode(unit->position);
            unit->team = TeamHumans;
            units.append(unit);
        }

        if(units.count() == 0) {
            gameOver = true;
        }
    } // endif gameover
    //    }


    QPainter painter;
    painter.begin(this);
    painter.beginNativePainting();
    painter.setRenderHint(QPainter::HighQualityAntialiasing, true);

    glClearColor(0.88f, 0.88f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    mainModelView = QMatrix4x4(); // reset
    // set up the main view (affects all objects)
    mainModelView.perspective(40.0, aspectRatio, 1.0, 150.0);
    mainModelView.lookAt(camera + offset,QVector3D(0,0,0) + offset,QVector3D(0.0,0.0,1.0));

    foreach(Entity *unit, units) {
        unit->draw(mainModelView);
    }
    foreach(Entity *bullet, bullets) {
        bullet->draw(mainModelView);
    }
    foreach(Entity *enemy, enemies) {
        enemy->draw(mainModelView);
    }
    foreach(Entity *building, buildings) {
        building->draw(mainModelView);
    }
    foreach(Node* node, nodes) {
        bool drawNode = false;
        foreach(Entity* aunit, allUnits) {
            foreach(Node* waypoint, aunit->waypoints) {
                if(waypoint == node) {
                    drawNode = true;
                }
            }
        }
        if(drawNode) {
            node->draw(mainModelView);
        }
    }
    testUnit->draw(mainModelView);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    painter.endNativePainting();
    if(drawHud) {
        foreach(Entity *aunit, allUnits) { // draw health bars
            // this function could probably have a few less calculations - some values may be set at resizeGL instead
            // all sizes are relative to the width and height of the screen to create a consistent experience on all devices
            // health bars are ugly and take up lots of screen space - health should probably be represented in a better way in the future
            // only on selection of units, for instance?
            if(aunit == selectedUnit) {
                qreal boxWidth = width() * 0.07; // how wide is the box?
                qreal boxHeight = width() * 0.008; // how tall is the box?
                qreal yOffset = width() * 0.05; // how far above do we print the box?
                qreal fillWidth = boxWidth * aunit->health / MaxHealth;
                //QVector3D position = mainModelView * aunit->position;
                QPoint projected = project(aunit->position);
                qreal strokeX = projected.x() - boxWidth / 2.0;
                qreal strokeY = projected.y() - yOffset;
                painter.setPen(QPen(QColor(10, 10, 10, 120))); // dark alpha
                painter.setBrush(QBrush(QColor(20, 30, 40, 100)));
                painter.drawRoundedRect((int)strokeX, (int)strokeY, boxWidth, boxHeight, 3, 3, Qt::AbsoluteSize); // a box above each unit
                qreal healthColor = 220 * aunit->health / MaxHealth; // a bit dark color :)
                if(width() > 800) { // fix for small screens
                    painter.setPen(QPen(QColor(20,30,40,100))); // thin stroke
                } else {
                    painter.setPen(QPen(QColor(0,0,0,0))); // no stroke
                }
                painter.setBrush(QBrush(QColor(220 - healthColor, healthColor, 10, 210))); // a color dependent on health
                painter.drawRoundedRect((int)strokeX + 2, (int)strokeY + 2, fillWidth - 4, boxHeight - 4, 3, 3, Qt::AbsoluteSize);
            }
        }
        ui->draw(&painter);
    }

    painter.setPen(Qt::blue);
    QString framesPerSecond;
    framesPerSecond.setNum(frames /(frametime.elapsed() / 1000.0), 'f', 2);
    painter.drawText(20, 40, framesPerSecond + " fps");
    painter.drawText(20, 60, "cursor: " + QString::number(pressCursor.x()) + ", " + QString::number(pressCursor.y()) + ", " + QString::number(pressCursor.z()));
    painter.drawText(20, 80, "Unit queue: " + QString::number(recruitqueue));
    painter.drawText(20, 100, "Enable HUD - H, Sounds - S");
    painter.drawText(width() - 200, 60, "score: " + QString::number(score));
    painter.drawText(width() - 200, 80, "enemies: " + QString::number(enemies.count()));
    painter.drawText(width() - 200, 100, "moveState: " + QString::number(selectedUnit->moveState));
    painter.drawText(width() - 200, 120, "collision: " + QString::number(collision));
    if(gameOver) {
        QFont font;
        font.setPixelSize(height() / 4);
        painter.setFont(font);
        painter.drawText(QRectF(width() / 4, height() / 4, width() / 2, height() / 2),Qt::AlignCenter,tr("Game\nOver!"));
    }
    //    painter.drawText(20,80,"Verts: " + QString::number(cannon->model->vertices[20]));
    //    painter.drawText(20, 80, "Verts: " + QString::number(cannon->vertices.first().x()));


    painter.end();


    swapBuffers();

    if (!(frames % 100)) {
        frametime.start();
        frames = 0;
    }
    frames ++;
    lastFrameTime = currentFrameTime;
}

void GLWidget::regenerateNodes() {
    nodes.clear();
    nodeNeighbors.clear();
    for(int i = -MapSize; i < MapSize; i++) {
        for(int j = -MapSize; j < MapSize; j++) {
            bool tooClose = false;
            QVector3D position((qreal)i * NodeSize, (qreal)j * NodeSize, 0);
            foreach(Entity* building, buildings){
                if((building->position - position).lengthSquared() < NodeSizeSquared) {
                    tooClose = true;
                }
            }
            if(!tooClose) {
                Node* node = new Node();
                node->position = position;
                nodes.append(node);
            }
        }
    }
    foreach(Node* node, nodes) {
        QList<Node*> neighbors;
        foreach(Node* possibleNeighbor, nodes) {
            if((node->position - possibleNeighbor->position).lengthSquared() <= NodeSizeDiagonal && possibleNeighbor != node) { // if the distance between nodes are 1x1, a diagonal nodeneighbor will be x^2 = 1^2 + 1^2 away (Pythagoras)
                neighbors.append(possibleNeighbor);
            }
        }
        nodeNeighbors.insert(node, neighbors);
    }
}

QList<Node*> GLWidget::findPath(Node* startNode, Node* goalNode, QList<Node*> avoid) {
    // bug fix - if one of the nodes for some strange reason does not exist (has been seen happening) - use a completely different node
    if(!nodes.contains(startNode)) {
        startNode = nodes.first();
    }
    if(!nodes.contains(goalNode)) {
        goalNode = nodes.first();
    }
    // end bug fix
    qDebug() << "Finding path from" << startNode->position << "to" << goalNode->position;
    // che
    QList<Node*> closedSet;
    QList<Node*> openSet;
    QHash<Node*, qreal> gscore;
    QHash<Node*, qreal> hscore;
    QHash<Node*, qreal> fscore;
    QHash<Node*, Node*> cameFrom; // 1st came from 2nd parameter
    openSet.append(startNode);
    gscore.insert(startNode, 0);
    hscore.insert(startNode, (goalNode->position - startNode->position).lengthSquared());
    fscore.insert(startNode, (goalNode->position - startNode->position).lengthSquared());
    while (openSet.count() > 0) {
        Node* x;
        qreal lowestFScore = 0;
        bool firstFScore = true;
        foreach(Node* node, openSet) {
            qreal curFscore = fscore.value(node);
            if(curFscore < lowestFScore || firstFScore) {
                x = node;
                lowestFScore = curFscore;
                firstFScore = false;
            }
        }
        if(x == goalNode) {
            // reconstruct path
            QList<Node*> path;
            Node* currentNode = goalNode; // start at the goal
            //            path.prepend(goalPosition); // this is no longer legal - we can't be where there are no nodes
            while(currentNode != startNode) { // if we're not there yet
                path.prepend(currentNode); // add the current node's position to the beginning of the list
                currentNode = cameFrom.value(currentNode); // find out where this node came from
            }
            //            path.prepend(startNode); // always add the startnode to begin with

            //            path.prepend(startPosition); // this is completely unecessary! We're already there!
            return path; // return our path
        }
        openSet.removeAll(x);
        closedSet.append(x);
        foreach(Node* y, nodeNeighbors.value(x)) {
            if(closedSet.contains(y) || avoid.contains(y)) { // if in closed set or we should avoid this node
                continue;
            }
            qreal tentativeGScore = 0;
            bool tentativeIsBetter = false;
            tentativeGScore = gscore[x] + (x->position - y->position).lengthSquared();
            if(!openSet.contains(y) && !closedSet.contains(y)) {
                openSet.append(y);
                tentativeIsBetter = true;
            } else if(tentativeGScore < gscore.value(y)) {
                tentativeIsBetter = true;
            }
            if(tentativeIsBetter) {
                cameFrom.insert(y, x);
                gscore.insert(y, tentativeGScore);
                hscore.insert(y, (goalNode->position - y->position).lengthSquared());
                fscore.insert(y, gscore[y] + hscore[y]);
            }
        }
    }
    QList<Node*> nonfunctional;
    nonfunctional.append(startNode);
    qDebug() << "Path not found!";
    return nonfunctional; // we failed to find a path, just return the point we're at
}
QPoint GLWidget::project(QVector3D position) {
    position =  mainModelView * position;
    qreal winX = width() * (position.x() + 1) / 2;
    qreal winY = height() - height() * (position.y() + 1) / 2;
    return QPoint((int)winX, (int)winY);
}

QVector3D GLWidget::unProject(int x, int y) {

    // project click down to plane
    // about the mathematics: http://www.opengl.org/sdk/docs/man/xhtml/gluUnProject.xml
    // mainModelView should be our modelview projection matrix
    QMatrix4x4 inv = mainModelView.inverted();
    qreal coordx = 2*(qreal) x / (qreal) width() - 1;
    qreal coordy = 2*(qreal) (height() - y) / (qreal) height() - 1;
    QVector4D nearPoint4 = inv * QVector4D(coordx, coordy, -1, 1); // winZ = 2 * 0 - 1 = -1
    QVector4D farPoint4 = inv * QVector4D(coordx, coordy, 1, 1); // win> = 2 * 1 - 1 = 1
    if(nearPoint4.w() == 0.0) {
        return QVector3D();
    }
    qreal w = 1.0/nearPoint4.w();
    QVector3D nearPoint = QVector3D(nearPoint4);
    nearPoint *= w;
    w = 1.0/farPoint4.w();
    QVector3D farPoint = QVector3D(farPoint4);
    farPoint *= w;
    QVector3D dir = farPoint - nearPoint;
    if (dir.z()==0.0) // if we are looking in a flat direction we won't hit the ground
        return QVector3D(0,0,0);

    qreal t = - nearPoint.z() / dir.z(); // how long it is to the ground
    QVector3D cursor = nearPoint + dir * t;
    return cursor;
}

Node* GLWidget::closestNode(QVector3D position) {
    // first we find the closest node to us
    qreal lowestDistance = 0;
    Node* foundNode;
    bool firstStartDistance = true;
    foreach(Node* node, nodes) {
        qreal startDistance = (node->position - position).lengthSquared();
        if(startDistance < lowestDistance || firstStartDistance) {
            lowestDistance = startDistance;
            foundNode = node;
            firstStartDistance = false;
        }
    }
    return foundNode;
}

// Suggested mouse/finger interactions:
//        select several units: Press and hold for one second, then drag
//   imp  select unit: tap (less than one second)
//   imp  move or attack: tap (after selecting own unit(s)) on ground or enemy
//   imp  move map: drag and drop anywhere, just don't hold finger down first
//        deselect: press and hold for one second
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    ui->convertMousePos(event->x(),event->y());
    if (event->button() == Qt::LeftButton) {
        if (ui->mouseClick()) {
            inUi=true;
            return;
        }
        inUi=false;

        if(gameOver) { // make sure we have had the game over text shown for 1.5 seconds
            qDebug() << lastFrameTime - gameOverTime;
            if(lastFrameTime - gameOverTime > 1.5) {
                resetGame();
            }
            return;
        }
        holdtime.restart();

        pressCursor = unProject(event->x(), event->y());
        dragCursor = pressCursor;
        dragStartPosition = event->pos();
        pressOffset = offset;
        testUnit->position = unProject(event->x(), event->y());
        lastDragOffset = offset;
    }
}
// Dragging events
void GLWidget::mouseMoveEvent(QMouseEvent* event) {
    ui->convertMousePos(event->x(),event->y());

    if (event->buttons() & Qt::LeftButton) {
        if (inUi) {
            ui->move();
            return;
        }
        // this should be improved. This method is not accurate.
        QVector3D currentCursor = unProject(event->x(), event->y());
        QVector3D currentDragOffset = offset;
        if(dragging) {
            offset -= (currentCursor - dragCursor) - (currentDragOffset - lastDragOffset); // offset is negative to get the "drag and drop"-feeling
            lastDragOffset = currentDragOffset;
            dragCursor = currentCursor;
        } else {
            if(holdtime.elapsed() > 1000) { // TODO: selection mode

            } else if((QVector3D(dragStartPosition) - QVector3D(event->pos())).length() > DragDropTreshold) { // if we have been dragging for more than ten pixels
                dragging = true;
            }
        }
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    ui->convertMousePos(event->x(),event->y());
    if (event->button() == Qt::LeftButton) {
        if (inUi) {
            ui->mouseRelease();
        }  else if(!dragging) {
            if(holdtime.elapsed() > 1000) { // TODO: selection mode
                if((QVector3D(dragStartPosition) - QVector3D(event->pos())).length() > DragDropTreshold) { // select several

                } else { // deselect all

                }
            } else {
                QVector3D cursorPosition = pressCursor;
                bool foundUnit = false;
                qreal lastLength = ClickRadius;
                QList<Entity*> allUnits;
                allUnits.append(enemies);
                allUnits.append(units);
                allUnits.append(buildings);
                foreach(Entity* aunit, allUnits)  { // did we click on an enemy?
                    qreal length = (cursorPosition - aunit->position).length();
                    if(length < ClickRadius && length < lastLength) {
                        if(aunit->team == TeamEnemies) {
                            selectedUnit->currentTarget = aunit;
                            selectedUnit->orders = Entity::OrderAttack;
                            selectedUnit->moveState = Entity::StateStopped; // we shall no longer use our moveTarget variable
                        } else if(aunit->team == TeamHumans) {
                            /*if (aunit->type == Entity::TypeUnit) */
                            selectedUnit = aunit;
                            aunit->select();

                        }

                        foundUnit = true;
                        lastLength = length;
                    }
                }
                Node* cursorNode = closestNode(cursorPosition);
                if(!foundUnit) { // if we didn't find anything, we assume that we want to move the selected unit
                    selectedUnit->currentTarget = NULL;
                    selectedUnit->setWaypoints(findPath(selectedUnit->positionNode, cursorNode)); // set the move target of the unit to this point
                    selectedUnit->orders = Entity::OrderMove;
                }
            }
        }
        dragging = false;
    }
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    qDebug() << "Key event" << event->key() << Qt::Key_S;
    switch(event->key()) {
    case Qt::Key_S:
        useSound = !useSound;
        break;
    case Qt::Key_H:
        drawHud = !drawHud;
        break;
    }
}

void GLWidget::recruitUnit(/*location ? */) {
    /*if (enoughCash)*/  //one of your buildings. For now, just build cannons.
    if (recruitqueue == 0) {
        recruittime.restart();
    }

    recruitqueue++;
    //cash-= price;
}

void GLWidget::resizeGL(int width, int height) {
    aspectRatio = (qreal) width / (qreal) height;
    ui->resize();
}
