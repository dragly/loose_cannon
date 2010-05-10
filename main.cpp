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

#include <QApplication>
#include <QMainWindow>
#include "mainwindow.h"
#include "glwidget.h"

int main(int argc, char ** argv )
{
    Q_INIT_RESOURCE(texture);
    QApplication a( argc, argv );
    a.setApplicationName( "loose_cannon" );
    MainWindow mw;
    mw.showFullScreen();
//    mw.show();
    return a.exec();
}
