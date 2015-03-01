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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QImage>
#include <QPainter>
#include "qrgb.h"
#include <QMessageBox>
#include <QVector2D>
#include <QDebug>
#include "math.h"
#include "about.h"

#define readBSett(name, def) qSett.value("bSett"#name, def).toBool()
#define mqGray(a) ((qRed(a) + qGreen(a) + qBlue(a)) / 3)

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    sett(new DialogSettings(this))
{
    ui->setupUi(this);
    bScanned = false;
    QSettings qSett;
    if(readBSett(StayOnTop, true)){
        Qt::WindowFlags flags = windowFlags();
        flags |= Qt:: WindowStaysOnTopHint;
        setWindowFlags(flags);
        //show();
    }
    sett->W = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit(0);
}

void MainWindow::on_actionSettings_triggered()
{
    sett->show();
}
void MainWindow::DetectFirst(){
    QString addr = ui->tActiveDirectory->text();
    qDebug() << "Selected dir: " << addr;
    if(addr.isEmpty()){
        QMessageBox msgBox(this);
        msgBox.setText("Please select a folder!");
        msgBox.exec();
        return;
    }
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.bmp";
    QDir dir(addr);
    QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);
    if(fileInfoList.size()<1){
        QMessageBox msgBox(this);
        msgBox.setText("The selected folder does not contain images!");
        msgBox.exec();
        return;
    }
    try{
        QFileInfo& FirstFile = fileInfoList[0];
        qDebug() << "First image: " << FirstFile.filePath();
        QPixmap Image(FirstFile.filePath());
        if(Image.isNull()){
            QMessageBox msgBox(this);
            msgBox.setText("Unable to open the first image!");
            msgBox.exec();
            return;
        }
        int H = Image.height();
        int W = Image.width();
        qDebug() << "Width: " << W << "Height: " << H;
        if(H<4 || W<4){
            QMessageBox msgBox(this);
            msgBox.setText("First image is too small!");
            msgBox.exec();
            return;
        }
        QSettings qSett;

        int Rows = round(double(ui->FinalSizeY->value()) / double(H));
        int Cols = round(double(ui->FinalSizeX->value()) / double(W));
        int iNums = floor(double(fileInfoList.size()) / double(Rows * Cols));

        if(readBSett(SmartSave, false) && iNums>1){
            int RR = Rows, CC = Cols, changes = (Rows + Cols) -2;
            QMap<int, double> Vari;
            QMap<int, QVector2D> Vari2;
            do{
                double res = double(fileInfoList.size()) / double(RR * CC);
                res = res - floor(res);
                Vari.insert(changes, res);
                Vari2.insert(changes, QVector2D(RR, CC));
                if(res!=0.0 && res<0.99){
                    if(RR>CC) RR--;
                    else CC--;
                }else{
                    Rows = RR;
                    Cols = CC;
                    break;
                }
                changes--;
            }while(changes>=0);
            if(Rows!=RR && Cols!=CC){
                QMap<int, double>::iterator vmaxi = Vari.begin();
                QMap<int, double>::iterator ffi;
                for(ffi = Vari.begin(); ffi!=Vari.end(); ++ffi){
                    if(vmaxi.value() < ffi.value())
                        vmaxi = ffi;
                }
                QVector2D VVari = Vari2[vmaxi.key()];
                if((floor(double(fileInfoList.size()) / double(VVari.x() * VVari.y())) / double(iNums)) < 4.2){
                    Rows = VVari.x();
                    Cols = VVari.y();
                }
            }
        }

        qDebug() << "Cal. cols = " << Cols << "Cal. rows = " << Rows;

        ui->NumCols->setValue(Cols);
        ui->NumRows->setValue(Rows);
        ui->FinalSizeX_2->setValue(Cols * W);
        ui->FinalSizeY_2->setValue(Rows * H);
        if(readBSett(SaveRatioOnPreview, false)){
            if(W>H){
                ui->previewField->setPixmap(Image.scaledToWidth(ui->previewField->width()));
            }else{
                ui->previewField->setPixmap(Image.scaledToHeight(ui->previewField->height()));
            }
        }else{
            ui->previewField->setPixmap(Image.scaled(ui->previewField->width(), ui->previewField->height()));
        }

        ui->pushButton->setDisabled(false);
        bScanned = true;

    }catch(...){
        QMessageBox msgBox(this);
        msgBox.setText("Unable to open the first image!");
        msgBox.exec();
        return;
    }
}

void MainWindow::on_toolButton_clicked()
{
    ui->pushButton->setDisabled(true);
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                 "/home",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    if(dir.isEmpty()){
        QMessageBox msgBox(this);
        msgBox.setText("Please select a folder!");
        msgBox.exec();
        return;
    }
    ui->tActiveDirectory->setText(dir);
    bScanned = false;

    QSettings qSett;
    if(readBSett(AutoCalculate, true)){
        DetectFirst();
    }else
        ui->pushButton->setDisabled(false);
}

void MainWindow::saveImg(QImage& Image, QString What){
    QSettings qSett;
    if(readBSett(AutoSave, true)){
        Image.save(ui->tActiveDirectory->text()+"/"+What);
    }else{
        QString file = QFileDialog::getSaveFileName(this, tr("Save image"), ui->tActiveDirectory->text(), "*.png");
        try{
            if(!file.isEmpty()) Image.save(file+".png");
        }catch(...){
            QMessageBox msgBox(this);
            msgBox.setText("Image not saved!");
            msgBox.exec();
            return;
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    if(!bScanned) DetectFirst();
    if(!bScanned) return;

    // І тут вже починається магія :)
    ui->progressBar->setValue(0);
    QString addr = ui->tActiveDirectory->text();
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.bmp";
    QDir dir(addr);
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);
    {
            QFileInfo& FirstFile = files[0];
            QImage Image(FirstFile.filePath());
            if(Image.isNull()){
                QMessageBox msgBox(this);
                msgBox.setText("Unable to open the first image!");
                msgBox.exec();
                return;
            }
            int H = ui->NumRows->value() * Image.height();
            int W = ui->NumCols->value() * Image.width();
            ui->FinalSizeX_2->setValue((W>ui->FinalSizeX->value())?ui->FinalSizeX->value():W);
            ui->FinalSizeY_2->setValue((H>ui->FinalSizeY->value())?ui->FinalSizeY->value():H);
    }
    int GH = ui->FinalSizeY_2->value() / ui->NumRows->value();
    int GW = ui->FinalSizeX_2->value() / ui->NumCols->value();

    int iNum = ceil((double)files.size() / (double)(ui->NumRows->value() * ui->NumCols->value()));
    qDebug() << "Need " << iNum << " textures!";
    int GI = 0, cur = 0;
    QSettings qSett;
    QString Result;
    while(iNum>0){
        int q, w;
        if(readBSett(SmartSave, true)){
            if(iNum>=4 && !readBSett(DisableAlphaMode, true)){
                QImage Final(ui->FinalSizeX_2->value(), ui->FinalSizeY_2->value(), QImage::Format_RGBA8888);
                for(q = 0; q < ui->NumRows->value(); ++q){
                    for(w = 0; w < ui->NumCols->value(); ++w){
                        if(GI>=files.size()) break;
                        try{
                            QImage Image(files[GI].filePath());
                            if(Image.isNull()) continue;
                            qDebug() << "... work with " << files[GI].filePath() << " is " << (1+GI) << " from " << files.size();
                            Image = Image.scaled(GW, GH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                            for(int x = 0; x < GW; ++x){
                                for(int y = 0; y < GH; ++y){
                                    QRgb C = Image.pixel(x, y);
                                    Final.setPixel(w*GW + x, q*GH + y, qRgba(mqGray(C), 0, 0, 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox(this);
                            msgBox.setText(QString("Have problems with %1").arg(files[GI].fileName()));
                            msgBox.exec();
                            return;
                        }
                        ++GI;
                        ui->progressBar->setValue(GI * 100 / files.size());
                    }
                    if(GI>=files.size()) break;
                }
                iNum--;
                for(q = 0; q < ui->NumRows->value(); ++q){
                    for(w = 0; w < ui->NumCols->value(); ++w){
                        if(GI>=files.size()) break;
                        try{
                            QImage Image(files[GI].filePath());
                            if(Image.isNull()) continue;
                            qDebug() << "... work with " << files[GI].filePath() << " is " << (1+GI) << " from " << files.size();
                            Image = Image.scaled(GW, GH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                            for(int x = 0; x < GW; ++x){
                                for(int y = 0; y < GH; ++y){
                                    QRgb C = Image.pixel(x, y);
                                    QRgb C2 = Final.pixel(w*GW + x, q*GH + y);
                                    Final.setPixel(w*GW + x, q*GH + y, qRgba(qRed(C2), mqGray(C), 0, 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox(this);
                            msgBox.setText(QString("Have problems with %1").arg(files[GI].fileName()));
                            msgBox.exec();
                            return;
                        }
                        ++GI;
                        ui->progressBar->setValue(GI * 100 / files.size());
                    }
                    if(GI>=files.size()) break;
                }
                iNum--;
                for(q = 0; q < ui->NumRows->value(); ++q){
                    for(w = 0; w < ui->NumCols->value(); ++w){
                        if(GI>=files.size()) break;
                        try{
                            QImage Image(files[GI].filePath());
                            if(Image.isNull()) continue;
                            qDebug() << "... work with " << files[GI].filePath() << " is " << (1+GI) << " from " << files.size();
                            Image = Image.scaled(GW, GH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                            for(int x = 0; x < GW; ++x){
                                for(int y = 0; y < GH; ++y){
                                    QRgb C = Image.pixel(x, y);
                                    QRgb C2 = Final.pixel(w*GW + x, q*GH + y);
                                    Final.setPixel(w*GW + x, q*GH + y, qRgba(qRed(C2), qGreen(C2), mqGray(C), 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox(this);
                            msgBox.setText(QString("Have problems with %1").arg(files[GI].fileName()));
                            msgBox.exec();
                            return;
                        }
                        ++GI;
                        ui->progressBar->setValue(GI * 100 / files.size());
                    }
                    if(GI>=files.size()) break;
                }
                iNum--;
                for(q = 0; q < ui->NumRows->value(); ++q){
                    for(w = 0; w < ui->NumCols->value(); ++w){
                        if(GI>=files.size()) break;
                        try{
                            QImage Image(files[GI].filePath());
                            if(Image.isNull()) continue;
                            qDebug() << "... work with " << files[GI].filePath() << " is " << (1+GI) << " from " << files.size();
                            Image = Image.scaled(GW, GH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                            for(int x = 0; x < GW; ++x){
                                for(int y = 0; y < GH; ++y){
                                    QRgb C = Image.pixel(x, y);
                                    QRgb C2 = Final.pixel(w*GW + x, q*GH + y);
                                    Final.setPixel(w*GW + x, q*GH + y, qRgba(qRed(C2), qGreen(C2), qBlue(C2), mqGray(C)));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox(this);
                            msgBox.setText(QString("Have problems with %1").arg(files[GI].fileName()));
                            msgBox.exec();
                            return;
                        }
                        ++GI;
                        ui->progressBar->setValue(GI * 100 / files.size());
                    }
                    if(GI>=files.size()) break;
                }
                iNum--;
                saveImg(Final, QString("FINAL_%1.png").arg(cur));
                cur++;
                continue;
            }else if(iNum>=3){
                QImage Final(ui->FinalSizeX_2->value(), ui->FinalSizeY_2->value(), QImage::Format_RGB32);
                for(q = 0; q < ui->NumRows->value(); ++q){
                    for(w = 0; w < ui->NumCols->value(); ++w){
                        if(GI>=files.size()) break;
                        try{
                            QImage Image(files[GI].filePath());
                            if(Image.isNull()) continue;
                            qDebug() << "... work with " << files[GI].filePath() << " is " << (1+GI) << " from " << files.size();
                            Image = Image.scaled(GW, GH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                            for(int x = 0; x < GW; ++x){
                                for(int y = 0; y < GH; ++y){
                                    QRgb C = Image.pixel(x, y);
                                    Final.setPixel(w*GW + x, q*GH + y, qRgb(mqGray(C), 0, 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox(this);
                            msgBox.setText(QString("Have problems with %1").arg(files[GI].fileName()));
                            msgBox.exec();
                            return;
                        }
                        ++GI;
                        ui->progressBar->setValue(GI * 100 / files.size());
                    }
                    if(GI>=files.size()) break;
                }
                iNum--;
                for(q = 0; q < ui->NumRows->value(); ++q){
                    for(w = 0; w < ui->NumCols->value(); ++w){
                        if(GI>=files.size()) break;
                        try{
                            QImage Image(files[GI].filePath());
                            if(Image.isNull()) continue;
                            qDebug() << "... work with " << files[GI].filePath() << " is " << (1+GI) << " from " << files.size();
                            Image = Image.scaled(GW, GH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                            for(int x = 0; x < GW; ++x){
                                for(int y = 0; y < GH; ++y){
                                    QRgb C = Image.pixel(x, y);
                                    QRgb C2 = Final.pixel(w*GW + x, q*GH + y);
                                    Final.setPixel(w*GW + x, q*GH + y, qRgb(qRed(C2), mqGray(C), 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox(this);
                            msgBox.setText(QString("Have problems with %1").arg(files[GI].fileName()));
                            msgBox.exec();
                            return;
                        }
                        ++GI;
                        ui->progressBar->setValue(GI * 100 / files.size());
                    }
                    if(GI>=files.size()) break;
                }
                iNum--;
                for(q = 0; q < ui->NumRows->value(); ++q){
                    for(w = 0; w < ui->NumCols->value(); ++w){
                        if(GI>=files.size()) break;
                        try{
                            QImage Image(files[GI].filePath());
                            if(Image.isNull()) continue;
                            qDebug() << "... work with " << files[GI].filePath() << " is " << (1+GI) << " from " << files.size();
                            Image = Image.scaled(GW, GH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                            for(int x = 0; x < GW; ++x){
                                for(int y = 0; y < GH; ++y){
                                    QRgb C = Image.pixel(x, y);
                                    QRgb C2 = Final.pixel(w*GW + x, q*GH + y);
                                    Final.setPixel(w*GW + x, q*GH + y, qRgb(qRed(C2), qGreen(C2), mqGray(C)));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox(this);
                            msgBox.setText(QString("Have problems with %1").arg(files[GI].fileName()));
                            msgBox.exec();
                            return;
                        }
                        ++GI;
                        ui->progressBar->setValue(GI * 100 / files.size());
                    }
                    if(GI>=files.size()) break;
                }
                iNum--;
                saveImg(Final, QString("FINAL_%1.png").arg(cur));
                cur++;
                continue;
            }
        }

        QImage Final(ui->FinalSizeX_2->value(), ui->FinalSizeY_2->value(), QImage::Format_RGB32);
        QPainter p(&Final);
        for(q = 0; q < ui->NumRows->value(); ++q){
            for(w = 0; w < ui->NumCols->value(); ++w){
                if(GI>=files.size()) break;
                    try{
                        QImage Image(files[GI].filePath());
                        if(Image.isNull()) continue;
                        qDebug() << "... work with " << files[GI].filePath() << " is " << (1+GI) << " from " << files.size();
                        Image = Image.scaled(GW, GH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);//.convertToFormat(QImage::Format_Indexed8);
                        /*for(int x = 0; x < GW; ++x){
                            for(int y = 0; y < GH; ++y){
                                Final.setPixel(w*GW + x, q*GH + y, qGray(Image.pixel(x, y)));
                            }
                        }*/
                        p.drawImage(w*GW, q*GH, Image);
                    }catch(...){
                        QMessageBox msgBox(this);
                        msgBox.setText(QString("Have problems with %1").arg(files[GI].fileName()));
                        msgBox.exec();
                        return;
                    }
                ++GI;
                ui->progressBar->setValue(GI * 100 / files.size());
            }
            if(GI>=files.size()) break;
        }
        iNum--;
        //Final = Final.convertToFormat(QImage::Format_Indexed8);
        saveImg(Final, QString("FINAL_%1.png").arg(cur));
        if(iNum==0){
            ui->previewField->setPixmap(QPixmap::fromImage(Final.scaled(ui->previewField->width(), ui->previewField->height())));
        }
        cur++;
    }
    double tmp = (double)files.size() / (double)(ui->NumRows->value() * ui->NumCols->value());
    iNum = (tmp - floor(tmp)) * (ui->NumRows->value() * ui->NumCols->value());
    tmp = double(iNum) / double(ui->NumCols->value());
    Result = QString("Last file have %1 full rows (%1x%2) and %3 in last row.").arg((int)floor(tmp)).arg(ui->NumCols->value()).arg((int)ceil((tmp - floor(tmp))*double(ui->NumCols->value())));
    QMessageBox msgBox(this);
    msgBox.setText(Result);
    msgBox.exec();
}

void MainWindow::on_actionOpen_dir_triggered()
{
    on_toolButton_clicked();
}

void MainWindow::on_actionGenerate_triggered()
{
    on_pushButton_clicked();
}

void MainWindow::on_actionAbout_triggered()
{
    (new About(this))->show();
}
