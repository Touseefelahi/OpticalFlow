#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetImage(QImage &image)
{
    ui->labelImage->setPixmap(QPixmap::fromImage(image));
    ui->labelImage->update();
}
