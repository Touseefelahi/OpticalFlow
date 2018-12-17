#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "visionworks.h"
#include <QMainWindow>
#include <QImage>



#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void SetImage(QImage &image);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
