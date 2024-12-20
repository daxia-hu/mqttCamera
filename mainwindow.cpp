/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtCore/QDateTime>
#include <QtMqtt/QMqttClient>
#include <QtWidgets/QMessageBox>
#include <QIntValidator>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("网络摄像机");
    m_client = new QMqttClient(this);
    m_client->setHostname("mq.tongxinmao.com");
    m_client->setPort(18830);
    camera = new QCamera(this);
    cameraViewFinder = new QCameraViewfinder(this);
    cameraImageCapture = new QCameraImageCapture(camera);
    cameraImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);
    ui->verticalLayout_2->addWidget(cameraViewFinder);
    camera->setViewfinder(cameraViewFinder);
    connect(cameraImageCapture,SIGNAL(imageCaptured(int,QImage)),this,SLOT(onImageCaptured(int,QImage)));
    connect(m_client, &QMqttClient::stateChanged, this, &MainWindow::updateLogStateChange);
    connect(m_client, &QMqttClient::disconnected, this, &MainWindow::brokerDisconnected);
    updateLogStateChange();
    timer.setInterval(60);
    connect(&this->timer,&QTimer::timeout,this,&MainWindow::on_timerout);
    m_client->setCleanSession(true);
    m_client->connectToHost();
    ui->pushButton->setEnabled(false);
    ui->lineEdit->setValidator(new QIntValidator(ui->lineEdit));
}
void MainWindow::on_timerout()
{
    cameraImageCapture->capture();
}
MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::onImageCaptured(int idx,QImage img)
{
   QPixmap pixMap = QPixmap::fromImage(img);
   pixMap = pixMap.scaled(600, 600, Qt::KeepAspectRatio);
   QBuffer buffer;
   buffer.open(QIODevice::ReadWrite);
   pixMap.save(&buffer,"jpg");
   QString topicStr = "monitor/topic"+ui->lineEdit->text();
   m_client->publish(topicStr, buffer.data(),0,0);
}
void MainWindow::on_buttonConnect_clicked()
{
    if (m_client->state() == QMqttClient::Disconnected) {
        m_client->setCleanSession(true);
        m_client->connectToHost();
    } else {
        m_client->disconnectFromHost();
    }
}

void MainWindow::on_buttonQuit_clicked()
{
    QApplication::quit();
}

void MainWindow::updateLogStateChange()
{
    const QString content = QDateTime::currentDateTime().toString()
                    + QLatin1String(": State Change")
                    + QString::number(m_client->state())
                    + QLatin1Char('\n');
}

void MainWindow::brokerDisconnected()
{
    // 服务器断开
    ui->label->clear();
    ui->label->setText("连接断开，请重新启动软件");
    timer.stop();
    camera->stop();
}

void MainWindow::setClientPort(int p)
{
}

void MainWindow::on_buttonPublish_clicked()
{
}

void MainWindow::on_buttonSubscribe_clicked()
{
}

void MainWindow::on_pushButton_clicked()
{
    if("打开摄像头" == ui->pushButton->text())
    {
        timer.start();
        camera->start();
        ui->pushButton->setText("关闭摄像头");
        ui->lineEdit->setEnabled(false);
    }
    else
    {
        ui->pushButton->setText("打开摄像头");
        camera->stop();
        timer.stop();
        ui->lineEdit->setEnabled(true);
        ui->label->clear();
    }
}

void MainWindow::on_pushButton_2_clicked()
{
}

void MainWindow::on_lineEdit_textChanged(const QString &arg1)
{
    if(ui->lineEdit->text() == "")
    {
        ui->pushButton->setEnabled(false);
    }
    else
    {
        ui->pushButton->setEnabled(true);
    }
}
