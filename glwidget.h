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
    void createGeometry();
    void quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4);
    void extrude(qreal x1, qreal y1, qreal x2, qreal y2);
    QTimer *timer;

    bool qtLogo;
    int frames;
    QTime time;
    QTime frametime;
    QGLShaderProgram program1;
    QGLShaderProgram program2;
    int vertexAttr1;
    int normalAttr1;
    int matrixUniform1;
    int texCoordAttr1;
    int textureUniform1;
    int vertexAttr2;
    int normalAttr2;
    int texCoordAttr2;
    int matrixUniform2;
    int textureUniform2;
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
    Model *cannon;
    Model *bullet;
    Model *enemy;
    qreal startAngle;
    qreal stopAngle;

    bool bulletFired;
    QVector3D bulletTarget;

    // mouse events
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    QPoint dragLastPosition;
    QPoint dragStartPosition;
    bool dragging;
    QTime dragtime;
};
#endif
