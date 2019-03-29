#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QMenu"
#include "QMenuBar"
#include "QAction"
#include "QFileDialog"
#include "about.h"
#include "cfgviewer.h"
#include "QDesktopServices"
#include "QSystemTrayIcon"
#include "QCloseEvent"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

signals:
    void openCfgMenuSelected(QString cfgContent);

private slots:
    void ssSelectFile();
    void openAbout();
    void setWebPageAddressToSsccCfg();
    void setServerAddressToSsccCfg();
    void getPort();
    void getDatas();
    void writeDatas();
    void restartSs();
    void closeSs();
    void changeSsConfig();
    void openSsCfgViewer();
    void openSsccCfgViewer();
    void openSsPath();
    void openSsccPath();
    void quit();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void getPortSetSsCfgAndRestartSs();

private:
    const QString cfgPath = "./sscc.txt";
    void checkCfgAndSsPath();
    void setDatas();
    void setSsConfigPath();
    void setDatasToVarAndWriteDatas();

    void closeEvent(QCloseEvent *event);

    About *about;
    CfgViewer *cfgViewer;

    Ui::MainWindow *ui;
    QString ssPath = "";
    QString ssConfigPath = "";
    QString webPageUrl = "";
    QString serverAddress = "";
    QString portStr = "";
    QString isAutoStart = "False";

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QAction *getPortAction;
    QAction *restartSsAction;
    QAction *closeSsAction;
    QAction *quitAction;
};

#endif // MAINWINDOW_H
