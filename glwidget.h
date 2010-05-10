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


// a
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtOpenGL>
#include <QtGui/qvector3d.h>
#include <QtGui/qmatrix4x4.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QTime>
#include <QVector>
#include <Phonon/MediaObject>
#include "model.h"

class Ui;
class Bubble;
class Cbutton;
class Node;
class Radar;
class Entity;
class Tank;
class SoundThread;

class GLWidget : public QGLWidget {

    Q_OBJECT

    friend class Ui;
    friend class Radar;
public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    enum Team {TeamHumans, TeamEnemies};
    QVector3D unProject(int x, int y);
    QVector3D unProject(int x, int y, QVector3D oldOffset);
    QPoint project(QVector3D position);

    static const qreal MaxHealth = 100.0;
    static const int MapSize = 30; // 2n x 2n nodes
    static const qreal NodeSize = 8; // each node is 8x8 m (the length of a tank)
    QList<Node*> findPath(Node* startPosition, Node* endPosition, QList<Node*> avoid = QList<Node*>());
    Node* closestNode(QVector3D position);
    // Sounds
    SoundThread *soundThread;
    QString sndExplosion;
    // end sounds
signals:
    void playSound(QString sample);
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
    void regenerateNodes();
    QTimer *timer;

    int score;

    int frames;
    QTime gametime;
    QTime frametime;
    QTime explosionSoundTime; // time since last explosion sound
    qreal aspectRatio;
    QVector3D camera;
    QVector3D pressCursor;
    QVector3D dragCursor;
    QVector3D offset;
    QVector3D pressOffset;
    QVector3D lastDragOffset;
    QMatrix4x4 mainModelView;
//    GLint gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
//                 const GLdouble model[16], const GLdouble proj[16],
//                 const GLint viewport[4],
//                 GLdouble * objx, GLdouble * objy, GLdouble * objz);
//    GLboolean invert_matrix(const GLdouble * m, GLdouble * out);
//    void matmul(GLdouble * product, const GLdouble * a, const GLdouble * b);
//    void transform_point(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);
    QList<Entity*> enemies;
    QList<Entity*> units;
    QList<Entity*> buildings;
    QList<Entity*> bullets;

    QList<Node*> nodes; // should probably have their own class - using Entity for convenience
    QHash<Node*, QList<Node*> > nodeNeighbors;

    QHash<Entity*, Entity*> bulletOwner;

    Entity* testUnit;
//    QHash<Entity*, QVector3D> bulletTargets;
    Entity* selectedUnit;
    Model *monkeyModel;
    Model *boxModel;
    Model *cannonModel;
    Model *humanTankBodyModel;
    Model *humanTankTowerModel;
    Model *enemyTankBodyModel;
    Model *enemyTankTowerModel;
    Model *bulletModel;
    Model *nodeModel;
    bool gameOver;
    qreal gameOverTime;
    qreal lastFrameTime;
    QPoint dragLastPosition;
    QPoint dragStartPosition;
    bool dragging;
    QTime holdtime;
    bool inUi;
    QTime recruittime; //vj: temp for test.    //bool recruiting;
    int recruitqueue;

    Ui* ui;

    // mouse events
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
public slots:
    void recruitUnit();
};
#endif
