/** SidleAnn: Program for automatic creation FlipBook-textures from set pictures
    Copyright (C) 2015 FrontPictures, OlexandrI
    https://github.com/OlexandrI/SidleAnn

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. **/

#include "about.h"
#include "ui_about.h"
#include <QSettings>

#define readBSett(name, def) qSett.value("bSett"#name, def).toBool()

About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About),
    MainW(0)
{
    ui->setupUi(this);
    QSettings qSett;
    if(readBSett(StayOnTop, true)){
        Qt::WindowFlags flags = windowFlags();
        flags |= Qt:: WindowStaysOnTopHint;
        setWindowFlags(flags);
    }
}

About::~About()
{
    delete ui;
}

void About::on_pushButton_clicked()
{
    this->close();
}
