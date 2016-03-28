#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "picstorage.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void onLoad();
	void onImport();
	void onSetRoot();
	void onAdd();
	void onMove();

protected:
	virtual void closeEvent(QCloseEvent *);
	virtual void showEvent(QShowEvent *);

private:
	Ui::MainWindow *ui;
	PicStorage* storage;
	QString mLastPath;
};

#endif // MAINWINDOW_H
