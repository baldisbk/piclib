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

	QString toString() const;
};

class PicStorage: public QObject
{
	Q_OBJECT
public:
	PicStorage();

public slots:
	void load(QString path);
	void setStorage(QString path);
	void import(QString path);

	void addFile(QString filename);
	void importFile(QString filename);
signals:
	void message(QString msg);
private:
	PicInfo* makeFromFile(QString fullpath);
	QMap<QString, PicInfo*> mStorage;
	QString mLocation;
};



#endif // PICSTORAGE_H
