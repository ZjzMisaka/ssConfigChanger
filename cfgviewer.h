#ifndef CFGVIEWER_H
#define CFGVIEWER_H

#include <QWidget>

namespace Ui {
class CfgViewer;
}

class CfgViewer : public QWidget
{
    Q_OBJECT

public:
    explicit CfgViewer(QWidget *parent = nullptr);
    ~CfgViewer();

private:
    Ui::CfgViewer *ui;

private slots:
    void showCfg(QString cfgContent);

};

#endif // CFGVIEWER_H
