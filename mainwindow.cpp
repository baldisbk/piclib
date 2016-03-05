#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	storage = new PicStorage;
	connect(storage, SIGNAL(message(QString)), ui->plainTextEdit, SLOT(appendPlainText(QString)));
	connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(onImport()));
	connect(ui->actionLoad, SIGNAL(triggered(bool)), this, SLOT(onLoad()));
//	connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(onImport()));
//	connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(onImport()));
//	connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(onImport()));
}

MainWindow::~MainWindow()
{
	delete ui;
	delete storage;
}

void MainWindow::onLoad()
{
	QString dn = QFileDialog::getExistingDirectory(this, "Load");
	if (!dn.isEmpty())
		storage->load(dn);
}

void MainWindow::onImport()
{
	QString dn = QFileDialog::getExistingDirectory(this, "Import");
	if (!dn.isEmpty())
		storage->import(dn);
}

void MainWindow::onSetRoot()
{
	QString dn = QFileDialog::getExistingDirectory(this, "Root");
	if (!dn.isEmpty())
		storage->setStorage(dn);
}

void MainWindow::onAdd()
{
	QString dn = QFileDialog::getOpenFileName(this, "Add");
	if (!dn.isEmpty())
		storage->addFile(dn);
}

void MainWindow::onMove()
{
	QString dn = QFileDialog::getOpenFileName(this, "ImportFile");
	if (!dn.isEmpty())
		storage->importFile(dn);
}
