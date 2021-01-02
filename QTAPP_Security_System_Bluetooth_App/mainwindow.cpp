#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QList>
#include <QDateTime>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    connect(this->discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(captureDeviceProperties(QBluetoothDeviceInfo)));

    connect(this->discoveryAgent, SIGNAL(finished()),
            this, SLOT(searchingFinished()));

    this->socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol, this);
    connect(this->socket, SIGNAL(connected()),
            this, SLOT(connectionEstablished()));
    connect(this->socket, SIGNAL(disconnected()),
            this, SLOT(connectionInterrupted()));
    connect(this->socket, SIGNAL(readyRead()),
            this, SLOT(socketReadyToRead()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButtonSearch_clicked()
{
    addToLogs("Searching devices");
    ui->comboBoxDevices->clear();
    ui->pushButtonSearch->setEnabled(false);
    this->discoveryAgent->start();
}

void MainWindow::addToLogs(QString message)
{
    QString currentDateTime = QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    ui->textEditLogs->append(currentDateTime + "\t" + message);
}

void MainWindow::sendMessageToDevice(QString message)
{
    if(this->socket->isOpen() && this->socket->isWritable()) {
      this->addToLogs("Wysysłam informacje do urządzeniaa " + message);
      this->socket->write(message.toStdString().c_str());
    } else {
      this->addToLogs("Nie mogę wysłać wiadomości. Połączenie nie zostało ustanowione!");
    }
}

void MainWindow::on_pushButtonConnect_clicked()
{
    QString comboBoxQString = ui->comboBoxDevices->currentText();
    QStringList portList = comboBoxQString.split(" ");
    QString deviceAddres = portList.last();

    static const QString serviceUuid(QStringLiteral("00001101-0000-1000-8000-00805F9B34FB"));
    this->socket->connectToService(QBluetoothAddress(deviceAddres),QBluetoothUuid(serviceUuid),QIODevice::ReadWrite);
    this->addToLogs("Rozpoczęto łączenie z urządzeniem o nazwie: " + portList.first() + " i adresie: " + deviceAddres);
}

void MainWindow::on_pushButtonDisconnect_clicked()
{
    this->addToLogs("Zamykam połączenie.");
    this->socket->disconnectFromService();
}

void MainWindow::readFromPort()
{
}

void MainWindow::on_pushButtonOpen_clicked()
{
    this->addToLogs("Włączam diodę.");
    this->sendMessageToDevice("1");
}

void MainWindow::captureDeviceProperties(const QBluetoothDeviceInfo &device)
{
    ui->comboBoxDevices->addItem(device.name() + " " + device.address().toString());
  //  this->addToLogs("Device: " + device.name() + " found, address: " + device.address().toString());
}

void MainWindow::searchingFinished()
{
    ui->pushButtonSearch->setEnabled(true);
}

void MainWindow::connectionEstablished()
{
  this->addToLogs("Połączenie ustanowione.");
}

void MainWindow::connectionInterrupted()
{
  this->addToLogs("Połączenie przerwane.");
}

void MainWindow::socketReadyToRead()
{
    while(this->socket->canReadLine()) {
      QString line = this->socket->readLine();

      QString terminator = "\r";
      int pos = line.lastIndexOf(terminator);

      this->addToLogs(line.left(pos));
    }
}