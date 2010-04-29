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
//    along with this program->  If not, see <http://www.gnu.org/licenses/>.
//
//    Some parts of the code might still be from Nokia's Qt examples
//    and are of course Copyright (C) Nokia and/or its subsidiary(-ies).

#include "model.h"
#include <QtOpenGL>
#include <QPainter>
#include "glwidget.h"
#include "ui/window.h"

Entity::Entity(Model *model) {
    initEntity(model);
    type = TypeUnit;
    this->menu = NULL;
}

Entity::Entity(Model *model, int type) {
    initEntity(model);
    this->type = type;
    this->menu = NULL;
}
void Entity::addMenuPoitner(Window* menu) {
    this->menu = menu;
}
void Entity::select() {
    if (this->menu != NULL)
        menu->show();
}

void Entity::initEntity(Model *model) {
    scale = QVector3D(1,1,1);
    position = QVector3D(0,0,0);
    velocity = QVector3D(0,0,0);
    rotation = QVector3D(0,0,0);
    health = GLWidget::MaxHealth;
    useMoveTarget = false;
    currentTarget = NULL;
    team = GLWidget::TeamHumans;
    this->model = model;
}

void Entity::setModel(Model *model) {
    this->model = model;
}

void Entity::draw(QMatrix4x4 modelview) {
    modelview.translate(position);
    modelview.rotate(rotation.z(), 0, 0, 1);
    modelview.rotate(rotation.y(), 0, 1, 0); // do x and y last
    modelview.rotate(rotation.x(), 1, 0, 0);
    modelview.scale(scale);
    model->draw(modelview);
}

Model::Model(QString filename) {
    texture = 0;
    vertexAttr = 0;
    normalAttr = 0;
    matrixUniform = 0;
    texCoordAttr = 0;
    textureUniform = 0;
    program = new QGLShaderProgram();
    load(filename);
}
void Model::setShaderProgram(QGLShaderProgram *program) {
    this->program = program;
    initShaderProgram();
}

// Model functions
void Model::load(QString filename) {
    model = glmReadOBJ(filename.toLatin1().data());
    if(model->numtexcoords < 1) {
        qWarning() << "Missing UV map.";
    }
    GLMgroup* group;
    group = model->groups;
    groups.clear();
    while (group) {
        ModelGroup modelGroup;
        modelGroup.triangles.clear();
        for(uint i = 0; i < group->numtriangles; i++) {
            ModelTriangle *triangle = new ModelTriangle();
            for(int j = 0; j < 3; j++) {
                QVector3D vector(model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 0],
                                 model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 1],
                                 model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 2]);
                triangle->vertices[j] = vector;
            }
            for(int j = 0; j < 3; j++) {
                QVector3D vector(model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 0],
                                 model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 1],
                                 model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 2]);
                triangle->normals[j] = vector;
            }
            if(model->numtexcoords > 0) {
                for(int j = 0; j < 3; j++) {
                    QVector3D vector(model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 0],
                                     model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 1],
                                     model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 2]);
                    triangle->texcoords[j] = vector;
                }
            }
            modelGroup.triangles.append(triangle);
        }
        groups.append(modelGroup);
        group = group->next;
    }
    qDebug() << "loading file";
}
bool Model::setShaderFiles(QString fragmentShader, QString vertexShader) {
    if(!setFragmentShaderFile(fragmentShader)) {
        qDebug() << "Model::setShaderFiles(): Failed to set fragment shader" << fragmentShader;
        return false;
    }
    if(!setVertexShaderFile(vertexShader)) {
        qDebug() << "Model::setShaderFiles(): Failed to set vertex shader" << vertexShader;
        return false;
    }
    if(!linkShaderProgram()) {
        qDebug() << "Model::setShaderFiles(): Failed to link shader program";
        return false;
    }
    if(!initShaderProgram()) {
        qDebug() << "Model::setShaderFiles(): Failed to init shader program";
        return false;
    }
    return  true;
}
bool Model::setFragmentShaderFile(QString filename) {
    if(!program->addShaderFromSourceFile(QGLShader::Fragment, filename)) {
        qWarning() << "Could not load shader file " + filename + ": " << program->log();
        return false;
    }
    return true;
}
bool Model::setVertexShaderFile(QString filename) {
    if(!program->addShaderFromSourceFile(QGLShader::Vertex, filename)) {
        qWarning() << "Could not load shader file " + filename + ": " << program->log();
        return false;
    }
    return true;
}
bool Model::linkShaderProgram() {
    if(!program->link()) {
        qWarning() << "Failed to link program:" << program->log();
        return false;
    }
    return true;
}

bool Model::initShaderProgram() {
    vertexAttr = program->attributeLocation("vertex");
    normalAttr = program->attributeLocation("normal");
    texCoordAttr = program->attributeLocation("texCoord");
    matrixUniform = program->uniformLocation("matrix");
    textureUniform = program->uniformLocation("tex");
    if(!texCoordAttr) {
        qDebug() << "Failed to find texCoordAttr";
        return false;
    }
    if(!normalAttr) {
        qDebug() << "Failed to find normalAttr";
        return false;
    }
    return true;
}

void Model::draw(QMatrix4x4 modelview) {
    program->bind();
    program->setUniformValue(matrixUniform, modelview);
////    program->setUniformValue(textureUniform, 0);    // use texture unit 0 - causes performance hit - doesn't appear to do anything
    glBindTexture(GL_TEXTURE_2D, texture);
    program->enableAttributeArray(vertexAttr);
    program->enableAttributeArray(normalAttr);
    program->enableAttributeArray(texCoordAttr);
    foreach(const ModelGroup grp, groups) {
        foreach(ModelTriangle *triangle, grp.triangles) {
            program->setAttributeArray(vertexAttr, triangle->vertices);
            program->setAttributeArray(texCoordAttr, triangle->texcoords);
            program->setAttributeArray(normalAttr, triangle->normals);
            glDrawArrays(GL_TRIANGLES, 0, 3); // only 3 vertices per triangle
        }
    }
    program->disableAttributeArray(vertexAttr);
    program->disableAttributeArray(normalAttr);
    program->disableAttributeArray(texCoordAttr);
    program->release();
}
void Model::setTexture(GLuint texture) {
    this->texture = texture;
}
