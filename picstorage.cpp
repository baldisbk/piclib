#include "picstorage.h"

#include <QDir>
#include <QProcess>
#include <QCryptographicHash>
#include <QCoreApplication>
#include <QDebug>

PicStorage::PicStorage()
{
}

void PicStorage::load(QString path)
{
	QDir dir(path);
	QStringList filters;
	filters << "*.jpg" << "*.jpeg" << "*.png";
	QStringList files = dir.entryList(filters, QDir::Files);
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (QString fn, files) addFile(path + "/" + fn);
	foreach (QString d, dirs) load(path + "/" + d);
}

void PicStorage::setStorage(QString path)
{
	QFileInfo fi(path);
	if (fi.exists())
		mLocation = fi.absoluteFilePath();
}

void PicStorage::import(QString path)
{
	QDir dir(path);
	QStringList filters;
	filters << "*.jpg" << "*.jpeg" << "*.png";
	QStringList files = dir.entryList(filters, QDir::Files);
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (QString fn, files) importFile(path + "/" + fn);
	foreach (QString d, dirs) import(path + "/" + d);
}

void PicStorage::addFile(QString filename)
{
	PicInfo* pi = makeFromFile(filename);
	if (!pi) return;
	mStorage.insert(pi->sha1, pi);
}

void PicStorage::importFile(QString filename)
{
	PicInfo* pi = makeFromFile(filename);
	if (!pi) return;
	pi->filepath = QString("%1/%2/%3/").
			arg(mLocation).
			arg(pi->camera).
			arg(pi->datetime.toString("yyyy-MM-dd"));
	QFile::rename(filename, pi->filepath+pi->filename);
	mStorage.insert(pi->sha1, pi);
}

PicInfo *PicStorage::makeFromFile(QString fullpath)
{
	QFileInfo fi(fullpath);
	if (!fi.exists()) {
		return NULL;
	}
	PicInfo* pi = new PicInfo;
	pi->filename = fi.fileName();
	pi->filepath = fi.absolutePath();

	QProcess prc;
	prc.start("exiv2", QStringList() << "-qPkyct" << fi.absoluteFilePath());
	if (!prc.waitForStarted() || !prc.waitForFinished()) {
		delete pi;
		return NULL;
	}

	QByteArray res = prc.readAll();
	QByteArray sha = QCryptographicHash::hash(res, QCryptographicHash::Sha1);
	pi->sha1 = QString::fromLatin1(sha.toBase64());
	QStringList lines = QString::fromLocal8Bit(res).split('\n');
	foreach(QString line, lines) {
		QStringList fields = line.split(QRegExp("\\s+"));
		if (fields.size() < 4) {
			continue;
		}
		QString name = fields.takeFirst();
		QString type = fields.takeFirst();
		QString size = fields.takeFirst();
		QString value = fields.join(" ");
		if (name == "Exif.Photo.DateTimeOriginal") {
			pi->datetime = QDateTime::fromString(value, "yyyy:MM:dd HH:mm:ss ");
		}
		if (name == "Exif.Image.Model")
			pi->camera = pi->camera + (pi->camera.isEmpty()?"":" ") + value;
		if (name == "Exif.Image.Make")
			pi->camera = value + (pi->camera.isEmpty()?"":" ") + pi->camera;
	}
	QCoreApplication::processEvents();
	emit message(pi->toString());

	return pi;
}

QString PicInfo::toString() const
{
	return QString("%1/%2:\t%3 %4 %5").
			arg(filepath).arg(filename).arg(sha1).
			arg(datetime.toString("yyyy-MM-dd")).arg(camera);
}
