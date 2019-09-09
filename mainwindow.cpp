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

    restartSsAction = new QAction("打开/重启ss", this);
    connect(restartSsAction, SIGNAL(triggered()), this, SLOT(restartSs()));
    closeSsAction = new QAction("关闭ss", this);
    connect(closeSsAction, SIGNAL(triggered()), this, SLOT(closeSs()));
    getPortAction = new QAction("更新端口配置并重启ss", this);
    connect(getPortAction, SIGNAL(triggered()), this, SLOT(getPortSetSsCfgAndRestartSs()));
    quitAction = new QAction("退出", this);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));

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
    connect(ui->pb_close, &QPushButton::clicked, this, &MainWindow::closeSs);

    connect(this,  SIGNAL(openCfgMenuSelected(QString)), cfgViewer, SLOT(showCfg(QString)));

    checkCfgAndImgAndSsPath();

    QIcon icon;
    QFile fileIconPath(iconPath);

    if (!fileIconPath.exists())
    {
        QStyle* style = QApplication::style();
        icon = style->standardIcon(QStyle::SP_MessageBoxWarning);
    }
    else
    {
        icon = QIcon(iconPath);
    }

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(icon);
    trayIcon->setToolTip("sscc--ss端口更新器");
    trayIcon->show();
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restartSsAction);
    trayIconMenu->addAction(closeSsAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(getPortAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayIconMenu);
    connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    this->setWindowIcon(icon);
    cfgViewer->setWindowIcon(icon);
    about->setWindowIcon(icon);
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
    case QSystemTrayIcon::MiddleClick:
        trayIcon->showMessage("服务器地址:当前端口", QString(serverAddress) + ":" + QString(portStr), QSystemTrayIcon::NoIcon, 5000);
        break;
    case QSystemTrayIcon::DoubleClick:
        this->show();
        break;
    default:
        break;
    }

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
        ui->label_status->setText("开始获取端口号");
        portStr = temp;
        ui->lineEdit_portnum->setText(portStr);
        writeDatas();
        ui->label_status->setText("成功获取端口号");
        trayIcon->showMessage("成功获取端口号", QString(serverAddress) + ":" + QString(portStr), QSystemTrayIcon::NoIcon, 5000);
    }
    else
    {
        ui->label_status->setText("获取的端口号不合法");
        trayIcon->showMessage("获取的端口号不合法", "端口获取网页地址必须以 \"http://\" 开头", QSystemTrayIcon::Warning, 5000);
        return;
    }

    if(ui->checkBox_isrestart_getport->isChecked() && !this->isHidden())
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

    ui->label_status->setText("打开/重启ss成功");
}

void MainWindow::getPortSetSsCfgAndRestartSs()
{
    getPort();
    changeSsConfig();
    restartSs();
}

void MainWindow::closeSs()
{
    QProcess process;
    process.execute("TASKKILL /IM Shadowsocks.exe /F");
    process.close();
}

void MainWindow::quit()
{
    QApplication::exit();
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
    ui->checkBox_autostart->setChecked(isAutoStart.contains("True"));
    ui->label_status->setText("成功设置数据");
}

void MainWindow::getDatas()
{
    QByteArray cfgContent[5];
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
    cfgContent[4] = file.readLine();

    ssPath = QString(cfgContent[0]);
    ssPath = ssPath.remove(0, 8).trimmed();

    setSsConfigPath();

    webPageUrl = QString(cfgContent[1]);
    webPageUrl = webPageUrl.remove(0, 12).trimmed();

    serverAddress = QString(cfgContent[2]);
    serverAddress = serverAddress.remove(0, 15).trimmed();

    portStr = QString(cfgContent[3]);
    portStr = portStr.remove(0, 9).trimmed();

    isAutoStart = QString(cfgContent[4]);
    isAutoStart = isAutoStart.remove(0, 13).trimmed();

    ui->label_status->setText("已获得更新器配置文件中的数据");

    setDatas();
}

void MainWindow::writeDatas()
{
    QFile fileCfg(cfgPath);
    if (!fileCfg.exists())
    {
        ui->label_status->setText("未找到更新器配置文件");
        checkCfgAndImgAndSsPath();
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
        QString isAutoStartLine(QString("IsAutoStart: ") + isAutoStart);

        txtOutput << ssPathLine << endl;
        txtOutput << webPageUrlLine << endl;
        txtOutput << serverAddressLine << endl;
        txtOutput << portStrLine << endl;
        txtOutput << isAutoStartLine << endl;

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

void MainWindow::checkCfgAndImgAndSsPath()
{
    QFile fileCfgDir(cfgDir);
    QFile fileCfgPath(cfgPath);
    QFile fileImgDir(imgDir);
    QFile fileIconPath(iconPath);

    QString msg = "";

    if (!fileImgDir.exists() || !fileIconPath.exists())
    {
        ui->label_status->setText("未找到图标图片文件 ");
        QDir dir;
        dir.mkdir(imgDir);
        if (!QFile::exists("./img/ssccicon.png"))
        {
            msg += "源图标图片不存在, 建议重新下载 ";
        }

        if(!QFile::copy("./img/ssccicon.png", iconPath))
        {
            msg += "图标图片复制失败, 建议重启软 ";
        }
    }

    if (!fileCfgDir.exists())
    {
        msg += "未找到更新器配置文件 ";
        QDir dir;
        dir.mkdir(cfgDir);
        fileCfgDir.open(QIODevice::WriteOnly);
        fileCfgDir.close();
    }

    if (!fileCfgPath.exists())
    {
        msg += "未找到更新器配置文件 ";
        fileCfgPath.open(QIODevice::WriteOnly);
        fileCfgPath.close();
    }
    else
    {
        getDatas();
    }
    QFile fileSsDefault("../Shadowsocks.exe");
    QFile fileSsCustomize(ssPath);
    if(!fileSsDefault.exists() && !fileSsCustomize.exists())
    {
        msg += "未找到Shadowsocks.exe ";
        ssSelectFile();
    }
    else if(fileSsDefault.exists())
    {
        ssPath = "../Shadowsocks.exe";
        msg += "成功确认更新器配置文件和ss路径 ";
    }
    else
    {
        msg += "成功确认更新器配置文件和ss路径 ";
    }

    ui->label_status->setText(msg);
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
    if(QDesktopServices::openUrl(QUrl("file:" + cfgDir, QUrl::TolerantMode)))
    {
        ui->label_status->setText("打开更新器路径成功");
    }
    else
    {
        ui->label_status->setText("打开更新器路径失败");
        checkCfgAndImgAndSsPath();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    cfgViewer->hide();
    about->hide();

    QString appName = QApplication::applicationName();      //程序名称
    QString appPath = QApplication::applicationFilePath();      // 程序路径
    appPath = appPath.replace("/","\\");
    QSettings *reg=new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    QString val = reg->value(appName).toString();               // 如果此键不存在，则返回的是空字符串

    if(ui->checkBox_autostart->isChecked())
    {
        if(!isAutoStart.contains("True"))
        {
            //设置自启动
            if(val != appPath)
                reg->setValue(appName,appPath);                             // 如果移除的话，reg->remove(applicationName);
            reg->deleteLater();
        }
        isAutoStart = "True";
    }
    else
    {
        if(!isAutoStart.contains("False"))
        {
            //取消自启动
            if(val != appPath)
                reg->remove(appName);                                   // 如果移除的话，reg->remove(applicationName);
            reg->deleteLater();
        }
        isAutoStart = "False";
    }

    this->hide();
}

MainWindow::~MainWindow()
{
    delete cfgViewer;
    delete about;
    writeDatas();
    delete ui;
}

