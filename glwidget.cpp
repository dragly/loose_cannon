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
#include "ui/ui.h"
#include <QPainter>
#include <QPaintEngine>
#include <Phonon/MediaObject>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "ui/window.h"
#include "ui/cbutton.h"

// constants
const qreal UnitSpeed = 3.0; // m/s
const qreal UnitAcceleration = 10.0; // m/s^2
const qreal UnitFrictionSide = 6.0;
const qreal UnitFrictionAll = 2.0;
const qreal EnemySpawnDistance = 15; // m
const qreal Dt = 0.01; // the timestep
const qreal RotateSpeed = 180; // degrees/s
const qreal BulletSpeed = 20; // m/s
const qreal NumberOfEnemies = 0;
const qreal ClickRadius = 2;

// map and nodes
const int mapSize = 30; // 2n x 2n nodes
const qreal nodeSize = 8; // each node is 8x8 m (the length of a tank)
const qreal nodeSizeSquared = nodeSize*nodeSize; // just for convenience
const qreal nodeSizeDiagonal = nodeSize*nodeSize + nodeSize*nodeSize; // just for convenience

// gui and interactions
const qreal DragDropTreshold = 20;

// weapon constants
const qreal ExplosionRadius = 3;
const qreal ExplosionDamage = 30;
const qreal ExplosionForce = 20;
const qreal FireDistance = 20;
const qreal BulletSpawnTime = 2;
QVector3D Gravity(0, 0, -10); // m/s^2
const qreal GLWidget::MaxHealth;
GLWidget::~GLWidget()
{
}

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    //    soundbank = new KGrSoundBank(2);
    //    expsound = soundbank->loadSound("sounds/bomb-02.wav");
    //    expsound2 = soundbank->loadSound("sounds/bomb-02.ogg");
    //    qDebug() << QSound::isAvailable();
    //    explosion = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("sounds/explosion-02.ogg"));
    //    explosion2 = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("sounds/bomb-02.ogg"));
    //    explosion3 = Phonon::createPlayer(Phonon::GameCategory, Phonon::MediaSource("sounds/bomb-03.ogg"));
    qsrand(time(NULL));
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);
    monkeyModel = new Model("monkey1.obj");
    boxModel = new Model("box.obj");
    cannonModel = new Model("cannon.obj");
    tankModel = new Model("tank-body.obj");
    tankModel->scale *= 0.5;
    bulletModel = new Model("bullet.obj");
    nodeModel = new Model("box.obj");
    // initial values
    camera = QVector3D(20, -20, 60);
    dragBool = false;
    explosionSoundTime.restart();
    // timer, should be set last, just in case
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->setInterval(1);
    timer->start();
}
void GLWidget::resetGame() {
    regenerateNodes();
    // init all to zero (also avoids memory failures)
    currentTime = 0.0;
    gameOver = false;
    dragging = false;
    inMenu = false;
    frames = 0;
    score = 0;
    gameOverTime = 0.0;
    offset *= 0;
    recruitqueue = 0;
    // end init all to zero
    gametime.start();
    bullets.clear();
    enemies.clear();
    Entity* cannon = new Entity(tankModel, Entity::TypeUnit);
    cannon->position = QVector3D(10,5,1);
    cannon->positionNode = closestNode(cannon->position);
    cannon->team = TeamHumans;
    selectedUnit = cannon;
    units.append(cannon);
    Entity* cannon2 = new Entity(tankModel, Entity::TypeUnit);
    cannon2->position = QVector3D(1,1,1);
    cannon2->positionNode = closestNode(cannon2->position);
    cannon2->team = TeamHumans;
    units.append(cannon2);
    Entity* building = new Entity(boxModel, Entity::TypeBuilding);
    building->position = QVector3D(-4,4,0);
    building->positionNode = closestNode(building->position);
    building->health = 1000;
    baseMenu = new Window(ui,0,0,0.2,0.2,Window::TopLeft,true,&building->position,true,"Base");
    btn = new Cbutton(baseMenu,QPointF(0.015,0.05),"Recruit unit");
    connect(btn, SIGNAL(btnClicked()),SLOT(recruitUnit()));
    building->addMenuPoitner(baseMenu);
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
}

void GLWidget::createEnemy() {
    qDebug() << "Creating enemy";
    Entity *enemy = new Entity(tankModel, Entity::TypeUnit);
    enemy->team = TeamEnemies;
    enemies.append(enemy);
    resetEnemy(enemy);
}

void GLWidget::initializeGL ()
{
    glClearColor(0.8f, 0.7f, 0.8f, 1.0f);
    // create and set shaders
    QGLShaderProgram *program = new QGLShaderProgram(this);
    program->addShaderFromSourceFile(QGLShader::Fragment, "fshader.glsl");
    program->addShaderFromSourceFile(QGLShader::Vertex, "vshader.glsl");
    program->link();
    monkeyModel->setShaderProgram(program);
    cannonModel->setShaderProgram(program);
    bulletModel->setShaderProgram(program);
    boxModel->setShaderProgram(program);
    nodeModel->setShaderProgram(program);
    tankModel->setShaderProgram(program);
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
    furTexture = bindTexture(QImage("fur.resized.jpg"));
    GLuint metalTexture;
    glGenTextures(1, &metalTexture);
    metalTexture = bindTexture(QImage("metal.small.jpg"));
    boxModel->setTexture(furTexture);
    monkeyModel->setTexture(furTexture);
    cannonModel->setTexture(metalTexture);
    bulletModel->setTexture(metalTexture);
    tankModel->setTexture(metalTexture);
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
    qreal newTime = gametime.elapsed() / 1000.0;
    // Let's do physics!
    // The physics are calculated without being affected by framerates (100 calculations per second).
    // First we check the time to see wether or not we need to recalculate physics.
    // If we do, we iterate through each timestep until we have caught up with the framerate.
    // This avoids missed collisions due to bad framerates, but causes the CPU to work a bit more
    while(currentTime <= newTime) { // let the physics catch up with the current time
        QList<Entity*> allUnits; // all units, including enemies and our own
        allUnits.append(enemies);
        allUnits.append(units);
        QList<Entity*> allDestructibles; // all destructible things - units and buildings
        allDestructibles.append(allUnits);
        allDestructibles.append(buildings);
        currentTime += Dt; // next timestep
        if(!gameOver) { // do logic
            foreach(Entity* enemy, enemies) {
                if(enemy->currentTarget != NULL) {
                    qreal currentDistance = (enemy->currentTarget->position - enemy->position).length();
                    foreach(Entity* unit, units) {
                        qreal distance = (unit->position - enemy->position).length();
                        if(currentDistance > distance) { // the enemy always selects the closest target
                            enemy->currentTarget = unit; // if we are closer, the enemy chooses to target us
                        }
                    }
                } else {
                    enemy->currentTarget = selectedUnit; // this should be the base, but we don't have a base yet
                }
            }

            foreach(Entity* bullet, bullets) { // let's see what our bullets are doing
                bool hitUnit = false;
                bullet->velocity += Gravity * Dt;
                bullet->position += bullet->velocity * Dt;
                qreal bulletAngleZ = atan2(bullet->velocity.y(),bullet->velocity.x()) * 180 / M_PI + 90;
                bullet->rotation.setZ(bulletAngleZ);
                qreal bulletAngleX = atan2(sqrt(pow(bullet->velocity.x(),2) + pow(bullet->velocity.y(),2)),
                                           bullet->velocity.z()) * 180 / M_PI + 90;
                bullet->rotation.setX(bulletAngleX);
                foreach(Entity* aunit, allUnits) {
                    QVector3D distance = bullet->position - aunit->position;
                    if(distance.length() < 1.5 && bullet->team != aunit->team) {
                        hitUnit = true;
                    }
                } // foreach enemy
                if(bullet->position.z() < 0 || hitUnit) {
                    //                    soundbank->play(expsound, 0);
                    // TODO: Add sound system.
                    // TODO: Animate explosion with sprites as seen here: http://news.developer.nvidia.com/2007/01/tips_strategies.html
                    foreach(Entity *hitUnit, allDestructibles) {
                        QVector3D distance = hitUnit->position - bullet->position;
                        if(distance.length() < ExplosionRadius) { // in explosion radius
                            qreal damage = ExplosionDamage * (distance.length() / ExplosionRadius); // the damage is relative to the distance
                            hitUnit->health -= damage;
                            score += damage;
                            if(hitUnit->currentTarget == NULL && // if we have not selected a target
                               hitUnit->useMoveTarget == false && // and we are not moving anywhere
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
                            } else if(hitUnit->type != Entity::TypeBuilding){
                                qreal velocityChange = ExplosionForce * damage / 100;
                                hitUnit->velocity += distance.normalized() * velocityChange; // make the explosion change the velocity in the direction of the blast
                            }
                        }
                    }
                    bullets.removeOne(bullet);
                } // endif hit
            } // end foreach bullets
            foreach(Entity* aunit, allUnits) { // for all units
                QList<Entity*> alreadyChecked;
                // collision detection (path finding)
                foreach(Entity* collideUnit, allUnits) {
                    if(collideUnit == aunit || alreadyChecked.contains(aunit))
                        continue; // if it is ourselves, we of course don't collide and we don't need to check both A & B and B & A.
                    alreadyChecked.append(collideUnit);
                    if((collideUnit->position - aunit->position).lengthSquared() < nodeSizeSquared) {
                        if(aunit->moveState == Entity::StateStopped && collideUnit->moveState == Entity::StateStopped) { // worst case scenario - they shouldn't come this far

                        }
                    }
                }
                // end collision detection
                // set the unit to attack its target (rotate/direction)
                QVector3D aunitdir;
                bool shallMove = false;
                if(aunit->useMoveTarget && aunit->waypoints.count() > 0) {
                    aunitdir = aunit->moveTarget - aunit->position;
                    shallMove = true;
                } else if(aunit->currentTarget != NULL) {
                    aunitdir = aunit->currentTarget->position - aunit->position;
                    shallMove = true;
                }
                qreal aunitAngle = atan2(aunitdir.y(),aunitdir.x()) * 180 / M_PI + 90;
                qreal difference = aunitAngle - aunit->rotation.z();
                // end direction calculations
                if(shallMove) {
                    bool doMovement = true;
                    while(difference > 180) difference -= 360;
                    while(difference < -180) difference += 360;
                    if(difference > 0) {
                        aunit->rotation.setZ(aunit->rotation.z() + RotateSpeed * Dt);
                        if(difference - RotateSpeed * Dt < 0) {
                            aunit->rotation.setZ(aunitAngle);
                            doMovement = true;
                        }
                    } else if(difference < 0) {
                        aunit->rotation.setZ(aunit->rotation.z() - RotateSpeed * Dt);
                        if(difference + RotateSpeed * Dt > 0) {
                            aunit->rotation.setZ(aunitAngle);
                            doMovement = true;
                        }
                    }
                    if(aunit->currentTarget != NULL) {
                        if((aunit->currentTarget->position - aunit->position).length() < FireDistance) {
                            //                            aunit->velocity *= 0;
                            doMovement = false;
                            aunit->useMoveTarget = false;
                            aunit->waypoints.clear();
                        } else {
                            if(!aunit->useMoveTarget) {
                                aunit->waypoints = findPath(aunit->positionNode, aunit->currentTarget->positionNode);
                                aunit->useMoveTarget = true;
                            }
                        }
                    }
                    if((aunit->position - aunit->waypoints.first()->position).lengthSquared() < nodeSizeSquared) {
                        if(aunit->waypoints.count() > 0) { // we still got somewhere to go
                            aunit->positionNode = aunit->waypoints.first();
                            if(aunit->waypoints.count() > 1) { // only remove if we have more than one waypoint left
                                aunit->waypoints.removeFirst();
                            }
                            qDebug() << "Going to:" << aunit->waypoints.first()->position;
                            doMovement = true;
                        } else { // we are too close and we have no more places to go - lets stop!
                            //                                aunit->velocity *= 0;
                            aunit->useMoveTarget = false; // we are no longer to move towards a target since we are already there!
                            doMovement = false;
                        }
                    }
                    QVector3D vectorAcceleration;
                    vectorAcceleration += - aunit->velocity * UnitAcceleration / UnitSpeed; // air resistance acts in the negative direction of the velocity
                    if(doMovement) { // only if we are moving, we will be affected by engine acceleration
                        qreal aunitAcceleration = UnitAcceleration; // acceleration (force of the enemy's engine) minus air resistance - set so that the equal eachother at ENEMY_SPEED
                        vectorAcceleration += aunitdir.normalized() * aunitAcceleration;
                    } // end doMovement
                    // side-friction
//                    if(aunit->position.z() == 0) {
//                        QVector3D projected = aunit->velocity - QVector3D::dotProduct(aunit->velocity, aunitdir.normalized()) * aunitdir.normalized();
//                        qreal aunitFriction = - UnitFrictionSide * projected.length(); // we find the amount of friction applied
//                        aunit->velocity += projected.normalized() * aunitFriction * Dt;	// apply the friction
//                    }
                    // all-way friction
                    vectorAcceleration += - aunit->velocity * UnitFrictionAll;
                    aunit->velocity += vectorAcceleration * Dt;
                } else { // else shallMove
                    aunit->velocity = QVector3D(0,0,aunit->velocity.z());
                } // end shallMove
                // Gravity
                if(aunit->position.z() > 0) {
                    aunit->velocity += Gravity * Dt;
                } else {
                    aunit->velocity.setZ(0);
                    aunit->position.setZ(0);
                }
                // fire bullets
                if(aunit->currentTarget != NULL && difference < 1 && difference > -1 && currentTime - aunit->lastBulletFired > BulletSpawnTime) {
                    if((aunit->currentTarget->position - aunit->position).length() < FireDistance) { // make sure we are close enough
                        Entity *bullet = new Entity(bulletModel, Entity::TypeBullet);
                        bullet->scale *= 0.3;
                        bullet->position = aunit->position + QVector3D(0,0,0.4);
                        QVector3D direction = aunit->currentTarget->position - bullet->position;
                        QVector3D calcTarget = aunit->currentTarget->position + aunit->currentTarget->velocity * direction.length() / BulletSpeed; // hit a bit ahead of target, suggesting same speed all the way
                        QVector3D calcDirection = calcTarget - bullet->position;
                        bullet->velocity = calcDirection.normalized() * BulletSpeed;
                        qreal bulletTime = calcDirection.length() / BulletSpeed;
                        qreal startSpeed = -Gravity.z() * bulletTime; // from v = v0 + at
                        bullet->velocity += QVector3D(0, 0, startSpeed * 0.5);
                        aunit->lastBulletFired = currentTime;
                        bullet->team = aunit->team;
                        bullet->type = Entity::TypeBullet;
                        bulletOwner.insert(bullet,aunit);
                        bullets.append(bullet);
                    }
                } // end fire bullets
                aunit->position += aunit->velocity * Dt; // do movement
            } // end foreach allUnits

            //recruitment of units //foreach all buildings and make the properties private or allocate them.
            if (recruitqueue > 0 && recruittime.elapsed() > 5000) {
                recruitqueue--;
                recruittime.restart();
                Entity* cannon = new Entity(tankModel, Entity::TypeUnit);
                cannon->position =/* aunit->position +*/ QVector3D(-4,6,6); //note, positioning might cause trouble with buildings at the edge of the map..
                cannon->positionNode = closestNode(cannon->position);
                cannon->team = TeamHumans;
                units.append(cannon);
            }

            if(units.count() == 0) {
                gameOver = true;
            }
        } // endif gameover
    }


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
    mainModelView.perspective(40.0, aspectRatio, 1.0, 100.0);
    mainModelView.lookAt(camera + offset,QVector3D(0,0,0) + offset,QVector3D(0.0,0.0,1.0));

    QList<Entity*> allUnits; // all units, including enemies and our own
    allUnits.append(enemies);
    allUnits.append(units);
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
    foreach(Entity* node, nodes) {
        bool drawNode = false;
        foreach(Entity* aunit, allUnits) {
            foreach(Entity* waypoint, aunit->waypoints) {
                if(waypoint == node) {
                    drawNode = true;
                }
            }
        }
        if(true) {
            node->draw(mainModelView);
        }
    }
    testUnit->draw(mainModelView);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    painter.endNativePainting();

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

    painter.setPen(Qt::blue);
    QString framesPerSecond;
    framesPerSecond.setNum(frames /(frametime.elapsed() / 1000.0), 'f', 2);
    painter.drawText(20, 40, framesPerSecond + " fps");
    painter.drawText(20, 60, "cursor: " + QString::number(pressCursor.x()) + ", " + QString::number(pressCursor.y()) + ", " + QString::number(pressCursor.z()));
    painter.drawText(20, 80, "Unit queue: " + QString::number(recruitqueue));
    painter.drawText(width() - 200, 60, "score: " + QString::number(score));
    painter.drawText(width() - 200, 80, "enemies: " + QString::number(enemies.count()));

    ui->draw(&painter);

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
}

void GLWidget::regenerateNodes() {
    nodes.clear();
    nodeNeighbors.clear();
    for(int i = -mapSize; i < mapSize; i++) {
        for(int j = -mapSize; j < mapSize; j++) {
            bool tooClose = false;
            QVector3D position((qreal)i * nodeSize, (qreal)j * nodeSize, 0);
            foreach(Entity* building, buildings){
                if((building->position - position).length() < 2) {
                    tooClose = true;
                }
            }
            if(!tooClose) {
                Entity* node = new Entity(boxModel);
                node->position = position;
                node->scale *= 0.3;
                nodes.append(node);
            }
        }
    }
    foreach(Entity* node, nodes) {
        QList<Entity*> neighbors;
        foreach(Entity* possibleNeighbor, nodes) {
            if((node->position - possibleNeighbor->position).lengthSquared() <= nodeSizeDiagonal) { // if the distance between nodes are 1x1, a diagonal nodeneighbor will be x^2 = 1^2 + 1^2 away (Pythagoras)
                neighbors.append(possibleNeighbor);
            }
        }
        nodeNeighbors.insert(node, neighbors);
    }
}

QList<Entity*> GLWidget::findPath(Entity* startNode, Entity* goalNode) {
    qDebug() << "Finding path from" << startNode->position << "to" << goalNode->position;
    // che
    QList<Entity*> closedSet;
    QList<Entity*> openSet;
    QHash<Entity*, qreal> gscore;
    QHash<Entity*, qreal> hscore;
    QHash<Entity*, qreal> fscore;
    QHash<Entity*, Entity*> cameFrom; // 1st came from 2nd parameter
    openSet.append(startNode);
    gscore.insert(startNode, 0);
    hscore.insert(startNode, (goalNode->position - startNode->position).lengthSquared());
    fscore.insert(startNode, (goalNode->position - startNode->position).lengthSquared());
    while (openSet.count() > 0) {
        Entity* x;
        qreal lowestFScore = 0;
        bool firstFScore = true;
        foreach(Entity* node, openSet) {
            qreal curFscore = fscore.value(node);
            if(curFscore < lowestFScore || firstFScore) {
                x = node;
                lowestFScore = curFscore;
                firstFScore = false;
            }
        }
        if(x == goalNode) {
            // reconstruct path
            QList<Entity*> path;
            Entity* currentNode = goalNode; // start at the goal
            //            path.prepend(goalPosition); // this is no longer legal - we can't be where there are no nodes
            while(currentNode != startNode) { // if we're not there yet
                path.prepend(currentNode); // add the current node's position to the beginning of the list
                currentNode = cameFrom.value(currentNode); // find out where this node came from
            }
            path.prepend(startNode); // always add the startnode to begin with

            //            path.prepend(startPosition); // this is completely unecessary! We're already there!
            return path; // return our path
        }
        openSet.removeAll(x);
        closedSet.append(x);
        foreach(Entity* y, nodeNeighbors.value(x)) {
            if(closedSet.contains(y)) {
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
    QList<Entity*> nonfunctional;
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

Entity* GLWidget::closestNode(QVector3D position) {
    // first we find the closest node to us
    qreal lowestDistance = 0;
    Entity* foundNode;
    bool firstStartDistance = true;
    foreach(Entity* node, nodes) {
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
            inMenu=true;
            return;
        }
        inMenu=false;

        if(gameOver) { // make sure we have had the game over text shown for 1.5 seconds
            qDebug() << currentTime - gameOverTime;
            if(currentTime - gameOverTime > 1.5) {
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

    }
}
// Dragging events
void GLWidget::mouseMoveEvent(QMouseEvent* event) {
    ui->convertMousePos(event->x(),event->y());

    if (event->buttons() & Qt::LeftButton && !inMenu) {
        // this should be improved. This method is not accurate.
        QVector3D currentCursor = unProject(event->x(), event->y());
        if(dragging) {
            if(!dragBool) {
                offset -= 2 * (currentCursor - dragCursor); // offset is negative to get the "drag and drop"-feeling
                dragBool = true;
            } else {
                dragBool = false;
            }
        } else {
            if(holdtime.elapsed() > 1000) { // TODO: selection mode

            } else if((QVector3D(dragStartPosition) - QVector3D(event->pos())).length() > DragDropTreshold) { // if we have been dragging for more than ten pixels
                dragging = true;
            }
        }
        dragCursor = currentCursor;
    }
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    ui->convertMousePos(event->x(),event->y());
    if (event->button() == Qt::LeftButton) {
        if (inMenu) {
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
                            selectedUnit->useMoveTarget = false; // we shall no longer use our moveTarget variable
                        } else if(aunit->team == TeamHumans) {
                            /*if (aunit->type == Entity::TypeUnit) */
                                selectedUnit = aunit;
                                aunit->select();

                        }

                        foundUnit = true;
                        lastLength = length;
                    }
                }
                Entity* cursorNode = closestNode(cursorPosition);
                if(!foundUnit) { // if we didn't find anything, we assume that we want to move the selected unit
                    selectedUnit->currentTarget = NULL;
                    selectedUnit->waypoints = findPath(selectedUnit->positionNode, cursorNode); // set the move target of the unit to this point
                    selectedUnit->useMoveTarget = true; // let's move!
                }
            }
        }
        dragging = false;
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
    baseMenu->resize();
    btn->resize();
}
