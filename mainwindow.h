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

private:
    Ui::MainWindow *ui;
    DialogSettings* sett;

    bool bScanned;

    void DetectFirst();
    void saveImg(QImage& Image, QString What);
};

#endif // MAINWINDOW_H
