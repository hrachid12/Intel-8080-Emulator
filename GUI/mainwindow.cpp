#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "../game.c"

#include <stdio.h>
#include <string.h>
#include <QFileDialog>
#include <QMessageBox>

char file[100];

char* QString2charp (const QString &myString)
{
    QByteArray ba = myString.toLocal8Bit();
    char *c_str2 = ba.data();
    return c_str2;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_play_clicked()
{
    MainWindow::close();
    Play(file);
}


void MainWindow::on_pushButton_browse_clicked()
{
    QString file_name = QFileDialog::getExistingDirectory(this, "Select a directory", "C://");
    ui->lineEdit_files->setText(file_name);
    memset(file, '\0', 100);
    strcpy(file, QString2charp(file_name));
}

