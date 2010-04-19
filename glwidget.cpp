/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"
#include "model.h"
#include <QPainter>
#include <QPaintEngine>
#include <math.h>
#include <stdio.h>
#include <string.h>

const qreal max_momentum = 40.0;
const qreal momentum_slowdown = 0.5;

GLWidget::~GLWidget()
{
}

GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent)
{
    qtLogo = true;
    frames = 0;
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAutoBufferSwap(false);
    cannon = new Model("monkey1.obj");
    bullet = new Model("monkey1.obj");
    enemy = new Model("monkey1.obj");
    bullet->scale *= 0.1;
    enemy->position = QVector3D(12,15,0);
    // initial values
    rotation.setX(0);
    rotation.setY(0);
    rotation.setZ(0);
    camera = QVector3D(5, -12, 30);
    // timer
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->setInterval(1);
    timer->start();
    time.start();
}

void GLWidget::resizeGL(int width, int height) {
    aspectRatio = (qreal) width / (qreal) height;
}

void GLWidget::initializeGL ()
{
    glClearColor(0.8f, 0.7f, 0.8f, 1.0f);
    GLuint texture;
    glGenTextures(1, &texture);
    texture = bindTexture(QImage(":/fur.resized.jpg"));
    cannon->setShaderFiles("fshader.glsl","vshader.glsl");
    cannon->setTexture(texture);
    bullet->setShaderFiles("fshader.glsl","vshader.glsl");
    bullet->setTexture(texture);
    enemy->setShaderFiles("fshader.glsl","vshader.glsl");
    enemy->setTexture(texture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}
qreal currentTime = 0.0;
void GLWidget::paintGL()
{
    qreal newTime = time.elapsed() / 1000.0;
    qreal dt = 0.001; // the timestep
    qreal rotateSpeed = 90 * dt;
    qreal bulletSpeed = 10;
    qreal enemySpeed = 3;
    QVector3D gravity(0, 0, -20);

    QPainter painter;
    painter.begin(this);

    // do physics
    // the physics are calculated without being affected by framerates
    while(currentTime <= newTime) { // let the physics catch up with the current time
        currentTime += dt; // next timestep
        qreal difference = stopAngle - cannon->rotation.z();
        while(difference > 180) difference -= 360;
        while(difference < -180) difference += 360;
        if(difference > 0) {
            cannon->rotation.setZ(cannon->rotation.z() + rotateSpeed);
            if(difference - rotateSpeed < 0) {
                cannon->rotation.setZ(stopAngle);
            }
        } else if(difference < 0) {
            cannon->rotation.setZ(cannon->rotation.z() - rotateSpeed);
            if(difference + rotateSpeed > 0) {
                cannon->rotation.setZ(stopAngle);
            }
        }
        // Bullet calculations
        if(bulletFired) {
            bullet->velocity += gravity * dt;
            bullet->position += bullet->velocity * dt;
            if(bullet->position.z() < 0) {
                qDebug() << "Boom!";
                QVector3D distance = bullet->position - enemy->position;
                if(distance.length() < 2) {
                    qDebug() << "Blast!";
                    enemy->velocity += QVector3D(0,0,10);
                }
                bulletFired = false;
                bulletTarget = cannon->position;
                bullet->position = cannon->position;
            }
        } else {
            if(bulletTarget != cannon->position && difference < 1 && difference > -1) {
                QVector3D direction;
                bulletFired = true;
                bullet->position = cannon->position + QVector3D(0,0,0.01);
                direction = bulletTarget - bullet->position;
                bullet->velocity = direction.normalized() * bulletSpeed;
                qreal bulletTime = direction.length() / bulletSpeed;
                qreal startSpeed = -gravity.z() * bulletTime; // from v = v0 + at
                qDebug() << "startspeed" << startSpeed;
                bullet->velocity += QVector3D(0, 0, startSpeed * 0.5);
                qDebug() << "startpos" << bullet->position;
                qDebug() << "direction" << direction;
                QVector3D enemydir= cannon->position - enemy->position;
                enemy->velocity = enemydir.normalized() * enemySpeed;
                qDebug() << "enemydir" << enemydir;
                qreal enemyAngle = atan2(enemydir.y(),enemydir.x()) * 180 / M_PI + 90;
                enemy->rotation.setZ(enemyAngle);
            }
        }
        // Enemy movement
        enemy->position += enemy->velocity * dt;
        if(enemy->position.z() > 0) {
            enemy->velocity += gravity * dt;
        } else {
            enemy->velocity.setZ(0);
            enemy->position.setZ(0);
        }
    }
    painter.beginNativePainting();

    glClearColor(0.9f, 0.85f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    mainModelView = QMatrix4x4(); // reset
    // set up the main view (affects all objects)
    mainModelView.perspective(40.0, aspectRatio, 1.0, 60.0);
    mainModelView.lookAt(camera,QVector3D(0,0,0),QVector3D(0.0,0.0,1.0));
    cannon->draw(mainModelView);
    bullet->draw(mainModelView);
    enemy->draw(mainModelView);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    painter.endNativePainting();


    painter.setPen(Qt::white);

    painter.drawText(20, 50, "momentum: " + QString::number(momentum.x()) + ", " + QString::number(momentum.y()) + ", " + QString::number(momentum.z()));
    painter.drawText(20, 60, "pos: " + QString::number(cursor.x()) + ", " + QString::number(cursor.y()) + ", " + QString::number(cursor.z()));

    painter.end();

    swapBuffers();
    frames ++;
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
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
