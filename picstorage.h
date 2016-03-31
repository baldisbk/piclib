#ifndef PICSTORAGE_H
#define PICSTORAGE_H

#include <QObject>
#include <QMap>
#include <QDateTime>

struct PicInfo {
	QString filename;
	QString filepath;
	QString sha1;
	QDateTime datetime;
	QString camera;
	QString fullname() const {return filepath + "/" + filename;}

	QString toString() const;
};

class PicStorage: public QObject
{
	Q_OBJECT
public:
	PicStorage();

	QString location() const;

public slots:
	void setStorage(QString path);

	void load(QString path);
	void import(QString path);

	void addFile(QString filename);
	void importFile(QString filename);

	void loadStorage(QString filename = QString());
	void saveStorage(QString filename = QString());

	int size() const;
	PicInfo *info(int index) const;

signals:
	void message(QString msg);
	void progressStart(int size, QString msg);
	void progressEnd();
	void progress(int p);

private:
	PicInfo* makeFromFile(QString fullpath);
	QStringList scanDir(QString path, QStringList *subPaths = 0);

	QMap<QString, PicInfo*> mStorage;
	QString mLocation;
};

#endif // PICSTORAGE_H
