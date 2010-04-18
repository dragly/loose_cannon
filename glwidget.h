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

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtOpenGL>
#include <QtGui/qvector3d.h>
#include <QtGui/qmatrix4x4.h>
#include <QtOpenGL/qglshaderprogram.h>
#include <QTime>
#include <QVector>
#include "group.h"

#include "glm.h"

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
    GLuint  m_uiTexture;
    QVector3D rotation;
    QVector3D momentum;
    void paintMonkey();
    void createGeometry();
    void quad(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4);
    void extrude(qreal x1, qreal y1, qreal x2, qreal y2);
    QTimer *timer;

    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<Group> groups;
    bool qtLogo;
    QList<Bubble*> bubbles;
    int frames;
    QTime time;
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
    QVector3D player;

    QMatrix4x4 mainModelView;
//    GLint gluUnProject(GLdouble winx, GLdouble winy, GLdouble winz,
//                 const GLdouble model[16], const GLdouble proj[16],
//                 const GLint viewport[4],
//                 GLdouble * objx, GLdouble * objy, GLdouble * objz);
//    GLboolean invert_matrix(const GLdouble * m, GLdouble * out);
//    void matmul(GLdouble * product, const GLdouble * a, const GLdouble * b);
//    void transform_point(GLdouble out[4], const GLdouble m[16], const GLdouble in[4]);
    QVector<QVector3D> convertToQVector(GLfloat* values, int size);

    GLMmodel *model;

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
