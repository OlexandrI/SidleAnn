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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <dialogsettings.h>
#include <qsettings.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionExit_triggered();

    void on_actionSettings_triggered();

    void on_toolButton_clicked();

    void on_pushButton_clicked();

    void on_actionOpen_dir_triggered();

    void on_actionGenerate_triggered();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;
    DialogSettings* sett;

    bool bScanned;

    void DetectFirst();
    void saveImg(QImage& Image, QString What);
};

#endif // MAINWINDOW_H
