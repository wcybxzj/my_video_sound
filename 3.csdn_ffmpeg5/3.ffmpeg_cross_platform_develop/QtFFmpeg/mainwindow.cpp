#include "mainwindow.h"
#include "ui_mainwindow.h"
extern "C"{
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    av_register_all();
    avcodec_register_all();

}

MainWindow::~MainWindow()
{
    delete ui;
}
