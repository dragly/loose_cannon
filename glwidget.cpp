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
#include <QPainter>
#include <QPaintEngine>
#include <math.h>
#include <stdio.h>
#include <string.h>

const qreal ENEMY_SPEED = 0.8; // units/s
const qreal ENEMY_ACCELERATION = 1.0; // units/s^2
const qreal ENEMY_FRICTION = 4.0;
const qreal ENEMY_SPAWNDISTANCE = 15; // units
const qreal DT = 0.01; // the timestep
const qreal ROTATE_SPEED = 180; // degrees/s
const qreal BULLET_SPEED = 8; // units/s
const qreal NUMBER_OF_ENEMIES = 5;
QVector3D GRAVITY(0, 0, -20); // units/s^2
const qreal CLICK_RADIUS = 2;

// weapon constants
const qreal EXPLOSION_RADIUS = 3;
const qreal EXPLOSION_DAMAGE = 30;
const qreal FIRE_DISTANCE = 7;
const qreal BULLET_SPAWNTIME = 2;

GLWidget::~GLWidget()
{
}

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    qsrand(time(NULL));
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);
    monkeyModel = new Model("monkey1.obj");
    boxModel = new Model("box.obj");
    cannonModel = new Model("cannon.obj");
    bulletModel = new Model("bullet.obj");
    // initial values
    camera = QVector3D(5, -7, 20);
    resetGame();
    // timer, should be set last, just in case
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->setInterval(1);
    timer->start();
}
void GLWidget::resetGame() {
    // init all to zero (also avoids memory failures)
    currentTime = 0.0;
    gameOver = false;
    dragging = false;
    frames = 0;
    score = 0;
    gameOverTime = 0.0;
    // end init all to zero
    gametime.start();
    bullets.clear();
    enemies.clear();
    Entity* cannon = new Entity(cannonModel);
    cannon->team = TeamHumans;
    selectedUnit = cannon;
    units.append(cannon);
    Entity* cannon2 = new Entity(cannonModel);
    cannon2->team = TeamHumans;
    units.append(cannon2);
    initEnemies();
}

void GLWidget::initEnemies() {
    for(int i = 0; i < NUMBER_OF_ENEMIES; i++) {
        createEnemy();
    }
}

void GLWidget::resetEnemy(Entity* enemy) {
    qreal randomAngle = qrand() * 360; // set random position
    enemy->position = QVector3D(cos(randomAngle * M_PI / 180) * ENEMY_SPAWNDISTANCE, sin(randomAngle * M_PI / 180) * ENEMY_SPAWNDISTANCE, 0.0); // set random position
    qDebug() << "Position enemy:" << enemy->position;
    enemy->health = 100; // reset health
    enemy->currentTarget = selectedUnit;
    enemy->team = TeamEnemies;
}

void GLWidget::createEnemy() {
    qDebug() << "Creating enemy";
    Entity *enemy = new Entity(cannonModel);
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
    // end textures

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
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
        currentTime += DT; // next timestep
        if(!gameOver) {

            foreach(Entity* enemy, enemies) {
                if(enemy->currentTarget != NULL) {
                    qreal currentDistance = (enemy->currentTarget->position - enemy->position).length();
                    foreach(Entity* unit, units) {
                        if(enemy->currentTarget) { // the enemy always selects the closest target

                        }
                    }
                } else {
                    enemy->currentTarget = selectedUnit; // this should be the base, but we don't have a base yet
                }
            }

            foreach(Entity* aunit, allUnits) {
                if(aunit->position.z() > 0) {
                    aunit->velocity += GRAVITY * DT;
                } else {
                    aunit->velocity.setZ(0);
                    aunit->position.setZ(0);
                }
                // set the unit to attack its target (rotate/direction)
                QVector3D aunitdir;
                bool shallMove = false;
                if(aunit->currentTarget != NULL) {
                    aunitdir = aunit->currentTarget->position - aunit->position;
                    shallMove = true;
                } else if(aunit->moveToTarget) {
                    aunitdir = aunit->moveTarget - aunit->position;
                    shallMove = true;
                }
                if(shallMove) {
                    qreal aunitAngle = atan2(aunitdir.y(),aunitdir.x()) * 180 / M_PI + 90;
                    qreal difference = aunitAngle - aunit->rotation.z();
                    bool readyToMove = false;
                    while(difference > 180) difference -= 360;
                    while(difference < -180) difference += 360;
                    if(difference > 0) {
                        aunit->rotation.setZ(aunit->rotation.z() + ROTATE_SPEED * DT);
                        if(difference - ROTATE_SPEED * DT < 0) {
                            aunit->rotation.setZ(aunitAngle);
                            readyToMove = true;
                        }
                    } else if(difference < 0) {
                        aunit->rotation.setZ(aunit->rotation.z() - ROTATE_SPEED * DT);
                        if(difference + ROTATE_SPEED * DT > 0) {
                            aunit->rotation.setZ(aunitAngle);
                            readyToMove = true;
                        }
                    }
                    if(aunit->currentTarget != NULL) {
                        if((aunit->currentTarget->position - aunit->position).length() < FIRE_DISTANCE) {
                            aunit->velocity *= 0;
                            readyToMove = false;
                        }
                    } else {
                        if((aunit->position - aunit->moveTarget).length() < 1) {
                            aunit->velocity *= 0;
                            readyToMove = false;
                        }
                    }
                    if(readyToMove) {
                        qreal aunitAcceleration = ENEMY_ACCELERATION - ENEMY_ACCELERATION * (aunit->velocity.length() / ENEMY_SPEED); // acceleration (force of the enemy's engine) minus air resistance - set so that the equal eachother at ENEMY_SPEED
                        aunit->velocity += aunitdir.normalized() * aunitAcceleration * DT;
                        // friction
                        if(aunit->position.z() == 0) {
                            QVector3D projected = aunit->velocity - QVector3D::dotProduct(aunit->velocity, aunitdir.normalized()) * aunitdir.normalized();
                            qreal aunitFriction = - ENEMY_FRICTION * projected.length(); // we find the amount of friction applied
                            aunit->velocity += projected.normalized() * aunitFriction * DT;	// apply the friction
                        }
                    } // end readyToMove
                    // fire bullets
                    if(aunit->currentTarget != NULL && difference < 1 && difference > -1 && currentTime - aunit->lastBulletFired > BULLET_SPAWNTIME) {
                        if((aunit->currentTarget->position - aunit->position).length() < FIRE_DISTANCE) { // make sure we are close enough
                            Entity *bullet = new Entity(bulletModel);
                            bullet->scale *= 0.3;
                            bullet->position = aunit->position + QVector3D(0,0,0.4);
                            QVector3D direction = aunit->currentTarget->position - bullet->position;
                            QVector3D calcTarget = aunit->currentTarget->position + aunit->currentTarget->velocity * direction.length() / BULLET_SPEED; // hit a bit ahead of target, suggesting same speed all the way
                            QVector3D calcDirection = calcTarget - bullet->position;
                            bullet->velocity = calcDirection.normalized() * BULLET_SPEED;
                            qreal bulletTime = calcDirection.length() / BULLET_SPEED;
                            qreal startSpeed = -GRAVITY.z() * bulletTime; // from v = v0 + at
                            bullet->velocity += QVector3D(0, 0, startSpeed * 0.5);
                            aunit->lastBulletFired = currentTime;
                            bullet->team = aunit->team;
                            bulletOwner.insert(bullet,aunit);
                            bullets.append(bullet);
                        }
                    } // end fire bullets
                } else { // else shallMove
                    aunit->velocity *= 0;
                } // end shallMove

                aunit->position += aunit->velocity * DT; // do movement
            }
        } // end foreach allUnits
        foreach(Entity* bullet, bullets) { // let's see what our bullets are doing
            bool hitUnit = false;
            bullet->velocity += GRAVITY * DT;
            bullet->position += bullet->velocity * DT;
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
                // TODO: Animate explosion with sprites as seen here: http://news.developer.nvidia.com/2007/01/tips_strategies.html
                foreach(Entity *hitUnit, allUnits) {
                    QVector3D distance = hitUnit->position - bullet->position;
                    if(distance.length() < EXPLOSION_RADIUS) { // in explosion radius
                        qreal damage = EXPLOSION_DAMAGE * (distance.length() / EXPLOSION_RADIUS); // the damage is relative to the distance
                        hitUnit->health -= damage;
                        score += damage;
                        if(hitUnit->currentTarget == NULL && hitUnit->team != bulletOwner[bullet]->team) {
                            hitUnit->currentTarget = bulletOwner[bullet]; // get the bastard who's shooting at us!
                        }
                        if(hitUnit->health < 0) { // we're dead
                            enemies.removeAll(hitUnit); // remove us from whatever list we came from
                            units.removeAll(hitUnit);
                            if(hitUnit->team == TeamEnemies) { // if it was an enemy, create a new one
                                createEnemy();
                            }
                            foreach(Entity* aunit, allUnits) { // if someone has us as a target, we're not target no more :(
                                if(aunit->currentTarget == hitUnit) {
                                    aunit->currentTarget = NULL;
                                }
                            }
                        } else {
                            qreal velocityChange = 2 * damage / 100;
                            hitUnit->velocity += distance.normalized() * velocityChange; // make the explosion change the velocity in the direction of the blast
                        }
                    }
                }
                bullets.removeOne(bullet);
            } // endif hit
        } // end foreach bullets
        if(units.count() == 0) {
            gameOver = true;
        }
    }


    QPainter painter;
    painter.begin(this);
    painter.beginNativePainting();

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
    mainModelView.perspective(40.0, aspectRatio, 1.0, 60.0);
    mainModelView.lookAt(camera,QVector3D(0,0,0),QVector3D(0.0,0.0,1.0));
    foreach(Entity *unit, units) {
        unit->draw(mainModelView);
    }
    foreach(Entity *bullet, bullets) {
        bullet->draw(mainModelView);
    }
    foreach(Entity *enemy, enemies) {
        enemy->draw(mainModelView);
    }
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    painter.endNativePainting();


    painter.setPen(Qt::blue);
    QString framesPerSecond;
    framesPerSecond.setNum(frames /(frametime.elapsed() / 1000.0), 'f', 2);
    painter.drawText(20, 40, framesPerSecond + " fps");
    painter.drawText(20, 60, "cursor: " + QString::number(cursor.x()) + ", " + QString::number(cursor.y()) + ", " + QString::number(cursor.z()));
    painter.drawText(width() - 200, 60, "score: " + QString::number(score));
    painter.drawText(width() - 200, 80, "enemies: " + QString::number(enemies.count()));
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
// Suggested mouse/finger interactions:
//        select several units: Press and hold for one second, then drag
//        select unit: tap (less than one second)
//        move or attack: tap (after selecting own unit(s)) on ground or enemy
//        move map: drag and drop anywhere, just don't hold finger down first
//        deselect: press and hold for one second
void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(gameOver) { // make sure we have had the game over text shown for 1.5 seconds
            qDebug() << currentTime - gameOverTime;
            if(currentTime - gameOverTime > 1.5) {
                resetGame();
            }
            return;
        }
        dragStartPosition = event->pos();
        dragtime.start();

        // project click down to plane
        // Another attempt
        // mainModelView should be our modelview projection matrix
        QMatrix4x4 inv = mainModelView.inverted();
        qreal coordx = (qreal) event->x() / (qreal) width();
        qreal coordy = (qreal) (height() - event->y()) / (qreal) height();
        // alright, don't ask me why, but we need to do this get the right position on screen
        coordx *= -2.0;
        coordx += 1.0;
        coordy *= -2.0;
        coordy += 1.0;
        // end workaround - one day we might need to fix this :)
        QVector3D screen = inv * QVector3D(coordx,coordy,-1);
        QVector3D center = inv * QVector3D(0, 0, 0);
        QVector3D dir = center - screen;
        if (dir.z()==0.0) // if we are looking in a flat direction we won't hit the ground
            return;

        qreal t = - (camera.z()) / dir.z(); // how long it is to the ground
        qDebug() << "t" << t;
        cursor.setX(camera.x() + dir.x() * t);
        cursor.setY(camera.y() + dir.y() * t);
        cursor.setZ(camera.z() + dir.z() * t); // should become zero

        bool foundUnit = false;
        qreal lastLength = CLICK_RADIUS;
        QList<Entity*> allUnits;
        allUnits.append(enemies);
        allUnits.append(units);
        foreach(Entity* aunit, allUnits)  { // did we click on an enemy?
            qreal length = (cursor - aunit->position).length();
            if(length < CLICK_RADIUS && length < lastLength) {
                if(aunit->team == TeamEnemies) {
                    selectedUnit->currentTarget = aunit;
                    selectedUnit->moveToTarget = false; // we shall no longer use our moveTarget variable
                } else if(aunit->team == TeamHumans) {
                    selectedUnit = aunit;
                }
                foundUnit = true;
                lastLength = length;
            }
        }
        if(!foundUnit) { // if we didn't find anything, we assume that we want to move the selected unit
            selectedUnit->currentTarget = NULL;
            selectedUnit->moveTarget = cursor; // set the move target of the unit to this point
            selectedUnit->moveToTarget = true; // let's move!
        }
    }
}
// Dragging events
void GLWidget::mouseMoveEvent(QMouseEvent* event) {
    if(!(event->buttons() & Qt::LeftButton))
        return;
    if(dragging) {
        int elapsed = dragtime.elapsed();
        QVector3D oldvalue = momentum;
        qreal relativey = (dragLastPosition.y() - event->pos().y()) / (qreal) height();
        qreal relativex = (event->pos().x() - dragLastPosition.x()) / (qreal) width();
        relativey *= 2500; // increase the factor
        relativex *= 2500;
        QVector3D vector = QVector3D(relativey / (qreal) elapsed, 0, relativex / (qreal) elapsed);
        momentum += vector;
        if (isinf(momentum.x()) || isnan(momentum.x())) {
            momentum.setX(oldvalue.x());
        }
        if (isinf(momentum.y()) || isnan(momentum.y())) {
            momentum.setY(oldvalue.y());
        }
        if (isinf(momentum.z()) || isnan(momentum.z())) {
            momentum.setZ(oldvalue.z());
        }
    }
    dragLastPosition = event->pos();
    dragging = true;
    dragtime.restart();
}
void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    dragging = false;

}

void GLWidget::resizeGL(int width, int height) {
    aspectRatio = (qreal) width / (qreal) height;
}
