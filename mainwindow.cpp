#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "htmlgetter.h"
#include <windows.h>

/*#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif*/

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    Qt::WindowFlags flag = nullptr;
    setWindowFlags(flag); // 设置禁止最大化
    setFixedSize(this->width(),this->height()); // 禁止改变窗口大小。

    ui->lineEdit_portnum->setStyleSheet("background-color: #DCDCDC");

    cfgViewer = new CfgViewer();
    about = new About();

    connect(ui->action_sssearch, SIGNAL(triggered()), this, SLOT(ssSelectFile()));
    connect(ui->action_about, SIGNAL(triggered()), this, SLOT(openAbout()));
    connect(ui->action_opensscfg, SIGNAL(triggered()), this, SLOT(openSsCfgViewer()));
    connect(ui->action_opensscccfg, SIGNAL(triggered()), this, SLOT(openSsccCfgViewer()));
    connect(ui->action_opensspath, SIGNAL(triggered()), this, SLOT(openSsPath()));
    connect(ui->action_openssccpath, SIGNAL(triggered()), this, SLOT(openSsccPath()));

    connect(ui->pb_setwebpageurl, &QPushButton::clicked, this, &MainWindow::setWebPageAddressToSsccCfg);
    connect(ui->pb_setserveraddress, &QPushButton::clicked, this, &MainWindow::setServerAddressToSsccCfg);
    connect(ui->pb_getport, &QPushButton::clicked, this, &MainWindow::getPort);
    connect(ui->pb_readfile, &QPushButton::clicked, this, &MainWindow::getDatas);
    connect(ui->pb_changessconfig, &QPushButton::clicked, this, &MainWindow::changeSsConfig);
    connect(ui->pb_writefile, &QPushButton::clicked, this, &MainWindow::setDatasToVarAndWriteDatas);
    connect(ui->pb_restart, &QPushButton::clicked, this, &MainWindow::restartSs);

    connect(this,  SIGNAL(openCfgMenuSelected(QString)), cfgViewer, SLOT(showCfg(QString)));

    checkCfgAndSsPath();
}



void MainWindow::setWebPageAddressToSsccCfg()
{
    webPageUrl = ui->lineEdit_webpageurl->text().trimmed();
    writeDatas();
    ui->label_status->setText("成功设置页面");
}

void MainWindow::setServerAddressToSsccCfg()
{
    serverAddress = ui->lineEdit_serveraddress->text().trimmed();
    writeDatas();
    ui->label_status->setText("成功设置地址");
}

void MainWindow::setDatasToVarAndWriteDatas()
{
    webPageUrl = ui->lineEdit_webpageurl->text().trimmed();
    serverAddress = ui->lineEdit_serveraddress->text().trimmed();
    portStr = ui->lineEdit_portnum->text().trimmed();
    writeDatas();
}

void MainWindow::setSsConfigPath()
{
    ssConfigPath = ssPath;
    if(ssConfigPath.contains("Shadowsocks.exe"))
    {
        ssConfigPath = ssConfigPath.remove(ssConfigPath.length() - 15, 15).trimmed();
    }
    else
    {
        ui->label_status->setText("设置ss配置文件路径失败, 建议重新选择ss路径");
        ssConfigPath = "";
        return;
    }
    if(!ssConfigPath.contains("gui-config.json"))
    {
        ssConfigPath = ssConfigPath.append("gui-config.json").trimmed();
    }
    ui->label_status->setText("成功设置ss配置文件路径");
}

void MainWindow::getPort()
{
    QString temp = HtmlGetter::getHtml(QString(webPageUrl));
    if(temp.toInt() >= 1 && temp.toInt() <= 65535)
    {
        portStr = temp;
        ui->lineEdit_portnum->setText(portStr);
        writeDatas();
        ui->label_status->setText("成功获取端口号");
    }
    else
    {
        ui->label_status->setText("获取的端口号不合法");
        return;
    }

    if(ui->checkBox_isrestart_getport->isChecked())
    {
        changeSsConfig();
        restartSs();
    }
}

void MainWindow::restartSs()
{
    QProcess process;
    process.execute("TASKKILL /IM Shadowsocks.exe /F");
    process.close();

    QProcess::startDetached(ssPath);

    ui->label_status->setText("重启ss成功");

}

void MainWindow::changeSsConfig()
{
    if(serverAddress == "" || portStr == "" || ui->lineEdit_serveraddress->text() == "" || ui->lineEdit_portnum->text() == "")
    {
        ui->label_status->setText("必要信息不完整, 更新失败");
        return;
    }

    QString strAll;
    QStringList strList;
    QFile readFile(ssConfigPath);
    if(readFile.open((QIODevice::ReadOnly|QIODevice::Text)))
    {
        ui->label_status->setText("正在读取ss配置文件");
        QTextStream stream(&readFile);
        strAll=stream.readAll();
    }
    else
    {
        ui->label_status->setText("打开并读取ss配置文件失败");
        return;
    }
    readFile.close();
    QFile writeFile(ssConfigPath);
    if(writeFile.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
        ui->label_status->setText("正在写入ss配置文件");
        QTextStream stream(&writeFile);
        strList=strAll.split("\n");
        for(int i=0; i<strList.count(); i++)
        {
            if(i==strList.count()-1)
            {
                //最后一行不需要换行
                stream<<strList.at(i);
            }
            else
            {
                stream<<strList.at(i)<<'\n';
            }

            if(strList.at(i).contains(serverAddress))
            {
                QString tempStr=strList.at(++i);
                tempStr.replace(0,tempStr.length(),"      \"server_port\": " + portStr + ",");
                stream<<tempStr<<'\n';
            }
        }
    }
    else
    {
        ui->label_status->setText("打开并写入ss配置文件失败");
        return;
    }
    writeFile.close();
    ui->label_status->setText("ss配置文件已更新");

    if(ui->checkBox_isrestart_changessconfig->isChecked())
        restartSs();
}

void MainWindow::setDatas()
{
    ui->lineEdit_webpageurl->setText(webPageUrl.trimmed());
    ui->lineEdit_serveraddress->setText(serverAddress.trimmed());
    ui->lineEdit_portnum->setText(portStr.trimmed());
    ui->label_status->setText("成功设置数据");
}

void MainWindow::getDatas()
{
    QByteArray cfgContent[4];
    QFile file(cfgPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->label_status->setText("打开更新器配置文件失败");
        return;
    }

    cfgContent[0] = file.readLine();
    cfgContent[1] = file.readLine();
    cfgContent[2] = file.readLine();
    cfgContent[3] = file.readLine();

    ssPath = QString(cfgContent[0]);
    ssPath = ssPath.remove(0, 8).trimmed();

    setSsConfigPath();

    webPageUrl = QString(cfgContent[1]);
    webPageUrl = webPageUrl.remove(0, 12).trimmed();

    serverAddress = QString(cfgContent[2]);
    serverAddress = serverAddress.remove(0, 15).trimmed();

    portStr = QString(cfgContent[3]);
    portStr = portStr.remove(0, 9).trimmed();

    ui->label_status->setText("已获得更新器配置文件中的数据");

    setDatas();
}

void MainWindow::writeDatas()
{
    QFile fileCfg(cfgPath);
    if (!fileCfg.exists())
    {
        ui->label_status->setText("未找到更新器配置文件");
        checkCfgAndSsPath();
    }
    else
    {
        if(!fileCfg.open(QIODevice::WriteOnly | QIODevice::Truncate))
        {
            ui->label_status->setText("打开更新器配置文件失败");
            return;
        }

        QTextStream txtOutput(&fileCfg);

        QString ssPathLine(QString("SsPath: ") + ssPath);
        QString webPageUrlLine(QString("WebPageUrl: ") + webPageUrl);
        QString serverAddressLine(QString("ServerAddress: ") + serverAddress);
        QString portStrLine(QString("PortStr: ") + portStr);

        txtOutput << ssPathLine << endl;
        txtOutput << webPageUrlLine << endl;
        txtOutput << serverAddressLine << endl;
        txtOutput << portStrLine << endl;

        fileCfg.close();

        ui->label_status->setText("写入更新器配置文件成功");

        setDatas();
    }
}

void MainWindow::ssSelectFile()
{
    ui->label_status->setText("选择SS路径");
    //定义文件对话框类
    QFileDialog *fileDialog = new QFileDialog(this);
    //定义文件对话框标题
    fileDialog->setWindowTitle(tr("选择SS路径"));
    //设置默认文件路径
    fileDialog->setDirectory("./");
    //设置文件过滤器
    fileDialog->setNameFilter(tr("exe(*.exe)"));
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //打印所有选择的文件的路径
    QStringList fileName;
    if(!fileDialog->exec())
    {
        ui->label_status->setText("没有选择有效文件");
        return;
    }
    fileName = fileDialog->selectedFiles();
    if(fileName[0].contains("Shadowsocks.exe"))
    {
        ssPath = fileName[0];
        setSsConfigPath();
        ui->label_status->setText("已选择有效文件: " + ssPath);
    }
    else
    {
        ui->label_status->setText("选择的文件名不合法 (未包含Shadowsocks.exe): " + fileName[0]);
        return;
    }

    writeDatas();
    return;
}

void MainWindow::checkCfgAndSsPath()
{
    QFile fileCfg(cfgPath);
    if (!fileCfg.exists())
    {
        ui->label_status->setText("未找到更新器配置文件");
        fileCfg.open(QIODevice::WriteOnly);
        fileCfg.close();
    }
    else
    {
        getDatas();
    }
    QFile fileSsDefault("../Shadowsocks.exe");
    QFile fileSsCustomize(ssPath);
    if(!fileSsDefault.exists() && !fileSsCustomize.exists())
    {
        ui->label_status->setText("未找到Shadowsocks.exe");
        ssSelectFile();
    }
    else if(fileSsDefault.exists())
    {
        ssPath = "./Shadowsocks.exe";
    }
    ui->label_status->setText("成功确认更新器配置文件和ss路径");
}

void MainWindow::openAbout()
{
    about->show();
}


void MainWindow::openSsCfgViewer()
{
    setSsConfigPath();
    cfgViewer->setWindowTitle("ss配置文件");
    cfgViewer->show();

    QString cfgContent;
    QFile file(ssConfigPath);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->label_status->setText("打开更新器配置文件失败");
        return;
    }

    cfgContent = file.readAll();

    emit openCfgMenuSelected(cfgContent);
}

void MainWindow::openSsccCfgViewer()
{
    cfgViewer->setWindowTitle("更新器配置文件");
    cfgViewer->show();

    QString cfgContent;
    QFile file(cfgPath);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->label_status->setText("打开更新器配置文件失败");
        return;
    }

    cfgContent = file.readAll();

    emit openCfgMenuSelected(cfgContent);
}

void MainWindow::openSsPath()
{
    QString filePath = ssPath.remove(ssPath.lastIndexOf('/'), ssPath.length() - ssPath.lastIndexOf('/'));
    if(QDesktopServices::openUrl(QUrl("file:" + filePath, QUrl::TolerantMode)))
    {
        ui->label_status->setText("打开ss路径成功");
    }
    else
    {
        ui->label_status->setText("打开ss路径失败, 建议重新选择ss路径");
    }
}

void MainWindow::openSsccPath()
{
    QString cfgPathTemp = cfgPath;
    QString filePath = cfgPathTemp.remove(cfgPathTemp.lastIndexOf('/'), cfgPathTemp.length() - cfgPathTemp.lastIndexOf('/'));
    if(QDesktopServices::openUrl(QUrl("file:" + filePath, QUrl::TolerantMode)))
    {
        ui->label_status->setText("打开更新器路径成功");
    }
    else
    {
        ui->label_status->setText("打开更新器路径失败");
        checkCfgAndSsPath();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

