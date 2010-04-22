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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtOpenGL>
#include <QtGui/qvector3d.h>
#include <QtGui/qmatrix4x4.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QTime>
#include <QVector>
#include "model.h"

class Bubble;
class GLWidget : public QGLWidget {

    Q_OBJECT
public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();
protected:
    void paintGL ();
    void initializeGL ();
    void resizeGL(int width, int height);
private:
    QVector3D rotation;
    QVector3D momentum;
    void createEnemy();
    void initEnemies();
    void resetGame();
    void resetEnemy(Entity* enemy);
    void fireBullet();
    QTimer *timer;

    int score;

    int frames;
    QTime time;
    QTime frametime;
    qreal aspectRatio;
    QVector3D camera;
    QVector3D cursor;

    QMatrix4x4 mainModelView;
//    GLint gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
//                 const GLdouble model[16], const GLdouble proj[16],
//                 const GLint viewport[4],
//                 GLdouble * objx, GLdouble * objy, GLdouble * objz);
//    GLboolean invert_matrix(const GLdouble * m, GLdouble * out);
//    void matmul(GLdouble * product, const GLdouble * a, const GLdouble * b);
//    void transform_point(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);
    Entity *cannon;
    QList<Entity*> enemies;
    QList<Entity*> bullets;
//    QHash<Entity*, QVector3D> bulletTargets;
    QVector3D currentBulletTarget;
    bool bulletFired;
    qreal lastBulletFired;
    Model *monkeyModel;
    Model *boxModel;
    Model *cannonModel;
    Model *bulletModel;
    QHash<Entity*, qreal> enemyHealth;
    qreal startAngle;
    qreal stopAngle;
    bool gameOver;
    qreal gameOverTime;
    qreal currentTime;
    QPoint dragLastPosition;
    QPoint dragStartPosition;
    bool dragging;
    QTime dragtime;

    // mouse events
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
};
#endif
