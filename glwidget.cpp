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

const qreal max_momentum = 40.0;
const qreal momentum_slowdown = 0.5;
qreal currentTime = 0.0;
const qreal enemySpeed = 1.2; // units/s
const qreal dt = 0.01; // the timestep
const qreal rotateSpeed = 180; // degrees/s
const qreal bulletSpeed = 15; // units/s

GLWidget::~GLWidget()
{
}

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);
    monkeyModel = new Model("monkey1.obj");
    boxModel = new Model("box.obj");
    cannonModel = new Model("cannon.obj");
    bulletModel = new Model("bullet.obj");
    cannon = new Entity(cannonModel);
    bullet = new Entity(bulletModel);
    bullet->scale *= 0.5;
    // initial values
    camera = QVector3D(5, -7, 20);
    time.start();
    resetGame();
    // timer, should be set last, just in case
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->setInterval(1);
    timer->start();
}
void GLWidget::resetGame() {
    // init all to zero (to avoid memory failures)
    gameOver = false;
    bulletFired = false;
    dragging = false;
    startAngle = 0;
    stopAngle = 0;
    frames = 0;
    score = 0;
    // end init all to zero
    enemies.clear();
    initEnemies();
}

void GLWidget::initEnemies() {
    for(int i = 0; i < 15; i++) {
        createEnemy();
    }
}

void GLWidget::resetEnemy(Entity* enemy) {
    qreal randomAngle = qrand() * 360; // set random position
    enemy->position = QVector3D(cos(randomAngle * M_PI / 180) * 20, sin(randomAngle * M_PI / 180) * 20, 0.0); // set random position
    qDebug() << "Position enemy:" << enemy->position;
    enemyHealth[enemy] = 100; // reset health
    // set the enemy to attack the cannon (rotate/direction)
    QVector3D enemydir= cannon->position - enemy->position;
    enemy->velocity = enemydir.normalized() * enemySpeed;
    qDebug() << "enemydir" << enemydir;
    qreal enemyAngle = atan2(enemydir.y(),enemydir.x()) * 180 / M_PI + 90;
    enemy->rotation.setZ(enemyAngle);
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
    qreal newTime = time.elapsed() / 1000.0;
    QVector3D gravity(0, 0, -30); // units/s^2
    // Let's do physics!
    // The physics are calculated without being affected by framerates (100 calculations per second).
    // First we check the time to see wether or not we need to recalculate physics.
    // If we do, we iterate through each timestep until we have caught up with the framerate.
    // This avoids missed collisions due to bad framerates, but causes the CPU to work a bit more
    if(!gameOver) {
        while(currentTime <= newTime) { // let the physics catch up with the current time
            currentTime += dt; // next timestep
            qreal difference = stopAngle - cannon->rotation.z();
            while(difference > 180) difference -= 360;
            while(difference < -180) difference += 360;
            if(difference > 0) {
                cannon->rotation.setZ(cannon->rotation.z() + rotateSpeed * dt);
                if(difference - rotateSpeed * dt < 0) {
                    cannon->rotation.setZ(stopAngle);
                }
            } else if(difference < 0) {
                cannon->rotation.setZ(cannon->rotation.z() - rotateSpeed * dt);
                if(difference + rotateSpeed * dt > 0) {
                    cannon->rotation.setZ(stopAngle);
                }
            }
            // Bullet calculations
            if(bulletFired) {
                bool hitOtherEnemy = false;
                bullet->velocity += gravity * dt;
                bullet->position += bullet->velocity * dt;
                qreal bulletAngleZ = atan2(bullet->velocity.y(),bullet->velocity.x()) * 180 / M_PI + 90;
                bullet->rotation.setZ(bulletAngleZ);
    //            qreal bulletAngleX = atan2(bullet->velocity.y(),bullet->velocity.z()) * 180 / M_PI + 90;
    //            bullet->rotation.setY(bulletAngleX);
                foreach(Entity *enemy, enemies) {
                    QVector3D distance = bullet->position - enemy->position;
                    if(distance.length() < 1.5) {
                        hitOtherEnemy = true;
                    }
                }
                if(bullet->position.z() < 0 || hitOtherEnemy) {
                    qDebug() << "Boom!";
                    foreach(Entity *enemy, enemies) {
                        QVector3D distance = bullet->position - enemy->position;
                        if(distance.length() < 3) { // in explosion radius
                            qDebug() << "Blast!";
                            enemyHealth[enemy] -= 40;
                            score += 100;
                            if(enemyHealth[enemy] < 0) {
                                resetEnemy(enemy); // reuse the one we've already got
                                createEnemy();
                            } else {
                                enemy->velocity += QVector3D(0,0,7);
                            }
                        }
                    }
                    bulletFired = false;
                    bulletTarget = cannon->position;
                    bullet->position = cannon->position;
                }
            } else {
                if(bulletTarget != cannon->position && difference < 1 && difference > -1) {
                    QVector3D direction;
                    bulletFired = true;
                    bullet->position = cannon->position + QVector3D(0,0,0.4);
                    direction = bulletTarget - bullet->position;
                    bullet->velocity = direction.normalized() * bulletSpeed;
                    qreal bulletTime = direction.length() / bulletSpeed;
                    qreal startSpeed = -gravity.z() * bulletTime; // from v = v0 + at
                    qDebug() << "startspeed" << startSpeed;
                    bullet->velocity += QVector3D(0, 0, startSpeed * 0.5);
                    qDebug() << "startpos" << bullet->position;
                    qDebug() << "direction" << direction;
                }
            }
            // Enemy movement
            foreach(Entity *enemy, enemies) {
                if((enemy->position - cannon->position).length() < 1.5) {
                    if(!gameOver) // makes sure we print this only once
                        qDebug() << "Game over!";
                    gameOver = true;
                } else {
                    enemy->position += enemy->velocity * dt;
                }
                if(enemy->position.z() > 0) {
                    enemy->velocity += gravity * dt;
                } else {
                    enemy->velocity.setZ(0);
                    enemy->position.setZ(0);
                }
                QVector3D enemydir= cannon->position - enemy->position;
                qreal enemyAngle = atan2(enemydir.y(),enemydir.x()) * 180 / M_PI + 90;
                enemy->rotation.setZ(enemyAngle);
            }
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
    cannon->draw(mainModelView);
    bullet->draw(mainModelView);
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
    painter.drawText(20, 80, "rotation: " + QString::number(cannon->rotation.x()) + ", " + QString::number(cannon->rotation.y()) + ", " + QString::number(cannon->rotation.z()));
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

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if(gameOver) {
            resetGame();
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
        // end workaround - one day we should fix this :)
        QVector3D screen = inv * QVector3D(coordx,coordy,-1);
        QVector3D center = inv * QVector3D(0, 0, 0);
        QVector3D dir = center - screen;
        // line is r = camera + t * dir

        if (dir.z()==0.0) // if we are looking in a flat direction
            return;

        qreal t = - (camera.z()) / dir.z(); // how long it is to the ground
        qDebug() << "t" << t;
        cursor.setX(camera.x() + dir.x() * t);
        cursor.setY(camera.y() + dir.y() * t);
        cursor.setZ(camera.z() + dir.z() * t); // should become zero

        //        if(!bulletFired) {
        bulletTarget = cursor;
        //        }
        // rotate towards the pointer
        QVector3D rcursor = cursor - cannon->position;
        stopAngle = atan2(rcursor.y(),rcursor.x()) * 180 / M_PI + 90;
        startAngle = cannon->rotation.z();
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
