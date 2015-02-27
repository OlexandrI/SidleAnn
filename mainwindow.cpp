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

#define readBSett(name, def) qSett.value("bSett"#name, def).toBool()

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    sett(new DialogSettings)
{
    ui->setupUi(this);
    bScanned = false;
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
        QMessageBox msgBox;
        msgBox.setText("Please select a folder!");
        msgBox.exec();
        return;
    }
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.bmp";
    QDir dir(addr);
    QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files|QDir::NoDotAndDotDot);
    if(fileInfoList.size()<1){
        QMessageBox msgBox;
        msgBox.setText("The selected folder does not contain images!");
        msgBox.exec();
        return;
    }
    try{
        QFileInfo& FirstFile = fileInfoList[0];
        qDebug() << "First image: " << FirstFile.filePath();
        QPixmap Image(FirstFile.filePath());
        if(Image.isNull()){
            QMessageBox msgBox;
            msgBox.setText("Unable to open the first image!");
            msgBox.exec();
            return;
        }
        int H = Image.height();
        int W = Image.width();
        qDebug() << "Width: " << W << "Height: " << H;
        if(H<4 || W<4){
            QMessageBox msgBox;
            msgBox.setText("First image is too small!");
            msgBox.exec();
            return;
        }
        QSettings qSett;

        int Rows = ui->FinalSizeY->value() / H;
        int Cols = ui->FinalSizeX->value() / W;

        if(readBSett(SmartSave, false)){
            int RR = Rows, CC = Cols, changes = (Rows + Cols) / 1.35;
            QMap<int, double> Vari;
            QMap<int, QVector2D> Vari2;
            do{
                double res = double(fileInfoList.size()) / double(RR * CC);
                res = res - floor(res);
                Vari.insert(changes, res);
                Vari2.insert(changes, QVector2D(RR, CC));
                if(res<0.95){
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
                Rows = VVari.x();
                Cols = VVari.y();
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
        QMessageBox msgBox;
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
        QMessageBox msgBox;
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
            QMessageBox msgBox;
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
                                    Final.setPixel(w*GW + x, q*GH + y, qRgba(qGray(C), 0, 0, 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox;
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
                                    Final.setPixel(w*GW + x, q*GH + y, qRgba(0, qGray(C), 0, 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox;
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
                                    Final.setPixel(w*GW + x, q*GH + y, qRgba(0, 0, qGray(C), 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox;
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
                                    Final.setPixel(w*GW + x, q*GH + y, qRgba(0, 0, 0, qGray(C)));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox;
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
            }else if(iNum==3){
                QImage Final(ui->FinalSizeX_2->value(), ui->FinalSizeY_2->value(), QImage::Format_RGB888);
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
                                    Final.setPixel(w*GW + x, q*GH + y, qRgb(qGray(C), 0, 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox;
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
                                    Final.setPixel(w*GW + x, q*GH + y, qRgb(0, qGray(C), 0));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox;
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
                                    Final.setPixel(w*GW + x, q*GH + y, qRgb(0, 0, qGray(C)));
                                }
                            }
                        }catch(...){
                            QMessageBox msgBox;
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
                        QMessageBox msgBox;
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
            Result = QString("Last file have %1x%2").arg(w).arg(q);
        }
        cur++;
    }
    /*QMessageBox msgBox;
    msgBox.setText(Result);
    msgBox.exec();*/
}
