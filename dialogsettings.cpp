#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include <QDebug>

#define readBSett(name, def) qSett.value("bSett"#name, def).toBool()
#define initBSett(name, def) ui->b##name->setChecked(qSett.value("bSett"#name, def).toBool())
#define saveBSett(name) qSett.setValue("bSett"#name, ui->b##name->isChecked())

DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);
    {
        QSettings qSett;
        initBSett(AutoCalculate, true);
        initBSett(AutoSave, true);
        initBSett(SmartSave, true);
        initBSett(SaveRatioOnPreview, false);
        initBSett(DisableAlphaMode, true);
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
    }
}

