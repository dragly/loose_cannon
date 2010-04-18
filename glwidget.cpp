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
const int bubbleNum = 8;

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
    model = new Model("monkey1.obj");
    // initial values
    rotation.setX(0);
    rotation.setY(0);
    rotation.setZ(0);
    camera = QVector3D(-5, -3, 20);
    // timer
    timer = new QTimer(this);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer->setInterval(1);
    timer->start();
}

void GLWidget::resizeGL(int width, int height) {
    aspectRatio = (qreal) width / (qreal) height;
}

void GLWidget::initializeGL ()
{
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    model->setFragmentShaderFile("fshader.glsl");
    model->setVertexShaderFile("vshader.glsl");
    model->linkShaderProgram();
    model->initShaderProgram();
    GLuint texture;
    glGenTextures(1, &texture);
    texture = bindTexture(QImage(":/fur.resized.jpg"));
    model->setTexture(texture);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
}

void GLWidget::paintGL()
{
    // do rotation - could we do this is one loop for xyz?
    if (momentum.z() > max_momentum) {
        momentum.setZ(max_momentum);
    } else if (momentum.z() < -max_momentum) {
        momentum.setZ(-max_momentum);
    }
    if(momentum.z() > 0) {
        momentum -= QVector3D(0, 0, momentum_slowdown * (qreal) time.elapsed() / 1000.0);
        if(momentum.z() < 0) {
            momentum.setZ(0);
        }
    }
    else if(momentum.z() < 0) {
        momentum += QVector3D(0, 0, momentum_slowdown * (qreal) time.elapsed() / 1000.0);
        if(momentum.z() > 0) {
            momentum.setZ(0);
        }
    }
    rotation += QVector3D(0,0,momentum.z() * 0.1);
    if (momentum.x() > max_momentum) {
        momentum.setX(max_momentum);
    } else if (momentum.x() < -max_momentum) {
        momentum.setX(-max_momentum);
    }
    if(momentum.x() > 0) {
        momentum -= QVector3D(momentum_slowdown * (qreal) time.elapsed() / 1000.0, 0, 0);
        if(momentum.x() < 0) {
            momentum.setX(0);
        }
    }
    else if(momentum.x() < 0) {
        momentum += QVector3D(momentum_slowdown * (qreal) time.elapsed() / 1000.0, 0, 0);
        if(momentum.x() > 0) {
            momentum.setX(0);
        }
    }
    rotation += QVector3D(momentum.x() * 0.1, 0,0);
    // end rotation

    //    createBubbles(bubbleNum - bubbles.count());

    QPainter painter;
    painter.begin(this);

    painter.beginNativePainting();

    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    //    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

    glFrontFace(GL_CW);
    glCullFace(GL_FRONT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    //    glDepthMask(GL_TRUE);
    mainModelView = QMatrix4x4(); // reset
    // set up the main view (affects all objects)
    mainModelView.perspective(60.0, aspectRatio, 1.0, 50.0);
    mainModelView.lookAt(camera,QVector3D(0,0,0),QVector3D(0.0,0.0,1.0));
    //    mainModelView.rotate(rotation.z(), 0.0, 0.0, 1.0);
    // inherit the main view for each object
    QMatrix4x4 mvMonkey = mainModelView;
    QMatrix4x4 mvMonkey2 = mainModelView;
    QMatrix4x4 mvMonkey3 = mainModelView;
    QMatrix4x4 mvMonkey4 = mainModelView;
    QMatrix4x4 mvMonkey5 = mainModelView;
    //    mvMonkey5.rotate(rotation.x(), 1.0, 0.0, 0.0);
    //    mvMonkey5.rotate(rotation.y(), 0.0, 1.0, 0.0);
    // do whatever with each object
    //    mvMonkey.rotate(90, 1, 0, 0);
    //    mvMonkey.scale(m_fScale * 2.0);
    mvMonkey.translate(-2.0,0,0);
    mvMonkey3.translate(2.0,0,0);
    mvMonkey4.translate(4.0,0,0);
    mvMonkey5.translate(player.x(),player.y(),player.z());
    mvMonkey5.rotate(rotation.z(), 0.0, 0.0, 1.0);
    model->draw(mvMonkey);
    model->draw(mvMonkey2);
    model->draw(mvMonkey3);
    model->draw(mvMonkey4);
    model->draw(mvMonkey5);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    painter.endNativePainting();

    QString framesPerSecond;
    framesPerSecond.setNum(frames /(time.elapsed() / 1000.0), 'f', 2);

    painter.setPen(Qt::white);

    painter.drawText(20, 40, framesPerSecond + " fps");
    painter.drawText(20, 50, "momentum: " + QString::number(momentum.x()) + ", " + QString::number(momentum.y()) + ", " + QString::number(momentum.z()));
    painter.drawText(20, 60, "pos: " + QString::number(player.x()) + ", " + QString::number(player.y()) + ", " + QString::number(player.z()));

    painter.end();

    swapBuffers();

    if (!(frames % 100)) {
        time.start();
        frames = 0;
    }
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
        qDebug() << "inv" << inv;
        qreal coordx = (qreal) event->x() / (qreal) width();
        qreal coordy = (qreal) (height() - event->y()) / (qreal) height();
        // alright, don't ask me why, but we need to do this get the right position on screen
        coordx *= -2.0;
        coordx += 1.0;
        coordy *= -2.0;
        coordy += 1.0;
        // end workaround - one day we should fix this :)
        QVector3D screen = inv * QVector3D(coordx,coordy,-1);
        qDebug() << "screen" << screen;
        QVector3D center = inv * QVector3D(0, 0, 0);
        qDebug() << "center" << center;
        QVector3D dir = center - screen;
        qDebug() << "direction" << dir;
        // line is r = camera + t * dir

        if (dir.z()==0.0) // if we are looking in a flat direction
            return;

        qreal t = - (camera.z()) / dir.z(); // how long it is to the ground
        qDebug() << "t" << t;
        player.setX(camera.x() + dir.x() * t);
        player.setY(camera.y() + dir.y() * t);
        player.setZ(camera.z() + dir.z() * t); // should become zero
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
