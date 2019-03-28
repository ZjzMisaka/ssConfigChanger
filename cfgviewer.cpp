#include "cfgviewer.h"
#include "ui_cfgviewer.h"
#include "QString"

CfgViewer::CfgViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CfgViewer)
{
    ui->setupUi(this);
    Qt::WindowFlags flag = nullptr;
    setWindowFlags(flag); // 设置禁止最大化
    setFixedSize(this->width(),this->height()); // 禁止改变窗口大小。
}

void CfgViewer::showCfg(QString cfgContent)
{
    ui->te_cfgtext->setText(cfgContent);
}

CfgViewer::~CfgViewer()
{
    delete ui;
}
