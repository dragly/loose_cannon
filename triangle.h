#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <QObject>
#include <QtOpenGL>

class Triangle
{
public:
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector3D> texcoords;
signals:

public slots:

};

#endif // TRIANGLE_H
