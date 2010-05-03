#ifndef NODE_H
#define NODE_H

#include <QtOpenGL>

class Node
{
public:
    Node();
    QVector3D position;

    void draw(QMatrix4x4 modelview);
};

#endif // NODE_H
