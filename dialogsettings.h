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

#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include <QDialog>
#include <qsettings.h>
class MainWindow;

namespace Ui {
class DialogSettings;
}

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSettings(QWidget *parent = 0);
    MainWindow* W;
    ~DialogSettings();

private slots:
    void on_buttonBox_accepted();

    void on_bStayOnTop_toggled(bool checked);

private:
    Ui::DialogSettings *ui;
};

#endif // DIALOGSETTINGS_H
