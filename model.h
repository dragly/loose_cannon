#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QtOpenGL>
#include <QPainter>
#include <QPaintEngine>

#include "glm.h"

class ModelTriangle
{
public:
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<QVector3D> texcoords;
signals:

public slots:

};

class ModelGroup
{
public:
    QVector<ModelTriangle> triangles;
signals:

public slots:

};

class Model
{
public:
    Model();
    Model(QString filename);

    // functions
    void load(QString filename);
    void draw(QMatrix4x4 modelview);
    void setTexture(GLuint texture);
    bool setVertexShaderFile(QString filename);
    bool setFragmentShaderFile(QString filename);
    void setProgram(QGLShaderProgram *program);
    bool linkShaderProgram();
    void initShaderProgram();
private:
    // variables
    GLMmodel *model;
    QVector<QVector3D> vertices;
    QVector<QVector3D> normals;
    QVector<ModelGroup> groups;
    GLuint texture;
    QGLShaderProgram program;
    int vertexAttr;
    int normalAttr;
    int matrixUniform;
    int texCoordAttr;
    int textureUniform;
};

#endif // MODEL_H
