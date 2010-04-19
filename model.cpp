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

#include "model.h"

Model::Model() {
    scale = QVector3D(1,1,1);
}

Model::Model(QString objectFile) {
    scale = QVector3D(1,1,1);
    load(objectFile);
}
void Model::load(QString filename) {
    model = glmReadOBJ(filename.toLatin1().data());
    if(model->numtexcoords < 1) {
        qWarning() << "Missing UV map.";
    }
    GLMgroup* group;
    group = model->groups;
    while (group) {
        ModelGroup grp;
        for(int i = 0; i < group->numtriangles; i++) {
            ModelTriangle triangle;
            QVector<QVector3D> verts;
            for(int j = 0; j < 3; j++) {
                QVector3D vector(model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 0],
                                 model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 1],
                                 model->vertices[3 * model->triangles[group->triangles[i]].vindices[j] + 2]);
                verts.append(vector);
            }
            QVector<QVector3D> norms;
            for(int j = 0; j < 3; j++) {
                QVector3D vector(model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 0],
                                 model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 1],
                                 model->normals[3 * model->triangles[group->triangles[i]].nindices[j] + 2]);
                norms.append(vector);
            }
            if(model->numtexcoords > 0) {
                QVector<QVector3D> texs;
                for(int j = 0; j < 3; j++) {
                    QVector3D vector(model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 0],
                                     model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 1],
                                     model->texcoords[2 * model->triangles[group->triangles[i]].tindices[j] + 2]);
                    texs.append(vector);
                }
                triangle.texcoords = texs;
            }
            triangle.vertices = verts;
            triangle.normals = norms;
            grp.triangles.append(triangle);
        }
        groups.append(grp);
        group = group->next;
    }
    qDebug() << "loading file";
}
bool Model::setShaderFiles(QString fragmentShader, QString vertexShader) {
    return setFragmentShaderFile(fragmentShader) && setVertexShaderFile(vertexShader) && linkShaderProgram() && initShaderProgram();;
}
bool Model::setFragmentShaderFile(QString filename) {
    if(!program.addShaderFromSourceFile(QGLShader::Fragment, filename)) {
        qDebug() << "Could not load shader file " + filename + ": " << program.log();
        return false;
    } else {
        qDebug() << "Loaded " + filename + " successfully";
        return true;
    }
}
bool Model::setVertexShaderFile(QString filename) {
    if(!program.addShaderFromSourceFile(QGLShader::Vertex, filename)) {
        qDebug() << "Could not load shader file " + filename + ": " << program.log();
        return false;
    } else {
        qDebug() << "Loaded " + filename + " successfully";
        return true;
    }
}
bool Model::linkShaderProgram() {
    if(program.link()) {
        qDebug() << "Program linked";
    } else {
        qDebug() << "Failed to link program:" << program.log();
    }
}

bool Model::initShaderProgram() {
    vertexAttr = program.attributeLocation("vertex");
    normalAttr = program.attributeLocation("normal");
    texCoordAttr = program.attributeLocation("texCoord");
    matrixUniform = program.uniformLocation("matrix");
    textureUniform = program.uniformLocation("tex");
    return true;
}

void Model::draw(QMatrix4x4 modelview) {
    modelview.translate(position);
    modelview.rotate(rotation.x(), 1, 0, 0);
    modelview.rotate(rotation.y(), 0, 1, 0);
    modelview.rotate(rotation.z(), 0, 0, 1);
    modelview.scale(scale);
    if(!program.bind()) {
        qDebug() << "Failed to bind program"; // Warning! If qDebug is removed from here, nothing is drawn on
        // screens of embedded devices. I have no idea why.
        // It is the strangest bug I have seen.
    }
    glBindTexture(GL_TEXTURE_2D, texture);
    foreach(ModelGroup grp, groups) {
        foreach(ModelTriangle triangle, grp.triangles) {
            program.setUniformValue(textureUniform, 0);    // use texture unit 0
            program.enableAttributeArray(normalAttr);
            program.enableAttributeArray(vertexAttr);
            program.enableAttributeArray(texCoordAttr);
            program.setAttributeArray(vertexAttr, triangle.vertices.constData());
            program.setAttributeArray(normalAttr, triangle.normals.constData());
            program.setAttributeArray(texCoordAttr, triangle.texcoords.constData());
            glDrawArrays(GL_TRIANGLES, 0, triangle.vertices.size());
            program.disableAttributeArray(normalAttr);
            program.disableAttributeArray(vertexAttr);
            program.disableAttributeArray(texCoordAttr);
        }
    }
    program.setUniformValue(matrixUniform, modelview);
    program.release();
}
void Model::setTexture(GLuint texture) {
    this->texture = texture;
}
