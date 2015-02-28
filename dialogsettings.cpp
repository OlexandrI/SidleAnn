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

#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include "mainwindow.h"
#include <QDebug>

#define readBSett(name, def) qSett.value("bSett"#name, def).toBool()
#define initBSett(name, def) ui->b##name->setChecked(qSett.value("bSett"#name, def).toBool())
#define saveBSett(name) qSett.setValue("bSett"#name, ui->b##name->isChecked())

DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings),
    W(0)
{
    ui->setupUi(this);
    QSettings qSett;
    {
        initBSett(AutoCalculate, true);
        initBSett(AutoSave, true);
        initBSett(SmartSave, true);
        initBSett(SaveRatioOnPreview, false);
        initBSett(DisableAlphaMode, true);
        initBSett(StayOnTop, true);
    }

    if(readBSett(StayOnTop, true)){
        Qt::WindowFlags flags = windowFlags();
        flags |= Qt:: WindowStaysOnTopHint;
        setWindowFlags(flags);
    }
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::on_buttonBox_accepted()
{
    {
        QSettings qSett;
        saveBSett(AutoCalculate);
        saveBSett(AutoSave);
        saveBSett(SmartSave);
        saveBSett(SaveRatioOnPreview);
        saveBSett(DisableAlphaMode);
        saveBSett(StayOnTop);
    }
}


void DialogSettings::on_bStayOnTop_toggled(bool checked)
{
    if(W && W->isVisible()){
        Qt::WindowFlags flags = W->windowFlags();
        if (checked) {
            flags |= Qt:: WindowStaysOnTopHint;
        }else{
            flags  &= ~Qt:: WindowStaysOnTopHint;
        }
        W->setWindowFlags(flags);
        W->show();
    }
}
