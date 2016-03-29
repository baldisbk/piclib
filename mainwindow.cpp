#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	storage = new PicStorage;
	mProgress = new QProgressDialog(this);
	mProgress->setWindowModality(Qt::WindowModal);
	mProgress->setMinimumDuration(1000);

	connect(storage, SIGNAL(message(QString)), ui->plainTextEdit, SLOT(appendPlainText(QString)));
	connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(onImport()));
	connect(ui->actionLoad, SIGNAL(triggered(bool)), this, SLOT(onLoad()));
	connect(ui->actionSetStorage, SIGNAL(triggered(bool)), this, SLOT(onSetRoot()));
//	connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(onImport()));
//	connect(ui->actionImport, SIGNAL(triggered(bool)), this, SLOT(onImport()));

	connect(storage, SIGNAL(progressStart(int,QString)), this, SLOT(showProgress(int,QString)));
	connect(storage, SIGNAL(progress(int)), this, SLOT(progress(int)));
	connect(storage, SIGNAL(progressEnd()), mProgress, SLOT(reset()));
}

MainWindow::~MainWindow()
{
	delete ui;
	delete storage;
}

void MainWindow::onLoad()
{
	QString dn = QFileDialog::getExistingDirectory(this, "Load", mLastPath);
	if (!dn.isEmpty())
		storage->load(mLastPath = dn);
}

void MainWindow::onImport()
{
	QString dn = QFileDialog::getExistingDirectory(this, "Import", mLastPath);
	if (!dn.isEmpty())
		storage->import(mLastPath = dn);
}

void MainWindow::onSetRoot()
{
	QString dn = QFileDialog::getExistingDirectory(this, "Root", storage->location());
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

void MainWindow::showProgress(int size, QString msg)
{
	mProgress->setLabelText(msg);
	mProgress->setRange(0, size);
	mProgress->setValue(0);
}

void MainWindow::progress(int p)
{
	mProgress->setValue(p);
}

void MainWindow::closeEvent(QCloseEvent *)
{
	QSettings settings;
	storage->saveStorage();
	settings.beginGroup("Paths");
	settings.setValue("last", mLastPath);
	settings.setValue("storage", storage->location());
	settings.endGroup();
}

void MainWindow::showEvent(QShowEvent *)
{
	QSettings settings;
	settings.beginGroup("Paths");
	QStringList defPaths = QStandardPaths::standardLocations(
		QStandardPaths::PicturesLocation);
	QString defPath = defPaths.isEmpty() ? QDir::homePath() : defPaths.first();
	mLastPath = settings.value("last", defPath).toString();
	storage->setStorage(settings.value("storage", defPath).toString());
	storage->loadStorage();
	settings.endGroup();
}
