#include "picstorage.h"

#include <QDir>
#include <QProcess>
#include <QCryptographicHash>
#include <QCoreApplication>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <QDebug>

PicStorage::PicStorage()
{
}

void PicStorage::load(QString path)
{
	QStringList files = scanDir(path);
	emit progressStart(files.size(), tr("Loading files..."));
	int i = 0;
	foreach (QString fn, files) {
		addFile(fn);
		emit progress(++i);
	}
	emit progressEnd();
}

void PicStorage::setStorage(QString path)
{
	QFileInfo fi(path);
	if (fi.exists()) {
		mLocation = fi.absoluteFilePath();
		emit message(QString("Storage set to %1").arg(mLocation));
	} else
		emit message(QString("Path %1 doesn't exist").arg(path));
}

void PicStorage::import(QString path)
{
	QStringList dirs;
	QStringList files = scanDir(path, &dirs);
	emit progressStart(files.size(), tr("Importing files..."));
	int i = 0;
	foreach (QString fn, files) {
		importFile(fn);
		emit progress(++i);
	}
	foreach(QString dir, dirs)
		QDir().rmdir(dir);

	emit progressEnd();
}

void PicStorage::addFile(QString filename)
{
	PicInfo* pi = makeFromFile(filename);
	if (!pi) return;
	if (mStorage.contains(pi->sha1)) {
		emit message(QString("File %1 already added").arg(pi->fullname()));
	}
	mStorage.insert(pi->sha1, pi);
	emit message(QString("File %1 added").arg(pi->fullname()));
}

void PicStorage::importFile(QString filename)
{
	PicInfo* pi = makeFromFile(filename);
	if (!pi) return;
	if (mStorage.contains(pi->sha1)) {
		emit message(QString("File %1 already added").arg(pi->fullname()));
		delete pi;
		return;
	}
	pi->filepath = QString("%1/%2/%3/").
			arg(mLocation).
			arg(pi->camera).
			arg(pi->datetime.toString("yyyy-MM-dd"));
	QDir().mkpath(pi->filepath);
	QFile::rename(filename, pi->fullname());
	emit message(QString("File %1 imported and moved to %2").arg(filename).arg(pi->fullname()));
	mStorage.insert(pi->sha1, pi);
}

void PicStorage::loadStorage(QString filename)
{
	if (filename.isEmpty())
		filename = mLocation + "/" + ".storage";
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		emit message(tr("Failed to open storage"));
		return;
	}

	mStorage.clear();

	QXmlStreamReader reader(&file);
	while (!reader.atEnd()) {
		switch (reader.readNext()) {
		case QXmlStreamReader::StartElement: {
			if (reader.name().toString() == "file") {
				PicInfo* pi = new PicInfo;
				pi->camera = reader.attributes().value("camera").toString();
				pi->datetime = QDateTime::fromString(
					reader.attributes().value("datetime").toString(),
					"yyyy:MM:dd HH:mm:ss");
				pi->filename = reader.attributes().value("file").toString();
				pi->filepath = reader.attributes().value("path").toString();
				pi->sha1 = reader.attributes().value("sha").toString();
				QString fullname = pi->fullname();
				QFileInfo fi(fullname);
				if (fi.exists()) {
					mStorage.insert(pi->sha1, pi);
				} else {
					emit message(QString("File %1 doesn't exist").arg(fullname));
					delete pi;
				}
			} else if (reader.name().toString() == "root") {
				QString root = reader.attributes().value("path").toString();
				if (!root.isEmpty())
					setStorage(root);
			}
		}
		default:;
		}
	}
	switch (reader.error()) {
	case QXmlStreamReader::NoError:
		emit message(tr("No error has occurred."));
		break;
	case QXmlStreamReader::NotWellFormedError:
		emit message(tr("The parser internally raised an error due to the read XML not being well-formed."));
		break;
	case QXmlStreamReader::PrematureEndOfDocumentError:
		emit message(tr("The input stream ended before a well-formed XML document was parsed."));
		break;
	case QXmlStreamReader::UnexpectedElementError:
		emit message(tr("The parser encountered an element that was different to those it expected."));
		break;
	default:;
	}
}

void PicStorage::saveStorage(QString filename)
{
	if (filename.isEmpty())
		filename = mLocation + "/" + ".storage";
	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		emit message(tr("Failed to open storage"));
		return;
	}

	QXmlStreamWriter stream(&file);
	stream.setAutoFormatting(true);
	stream.writeStartDocument();
	stream.writeStartElement("storage");
	stream.writeStartElement("file");
	stream.writeAttribute("path", mLocation);
	stream.writeEndElement();
	foreach(PicInfo* pi, mStorage) {
		stream.writeStartElement("file");
		stream.writeAttribute("camera", pi->camera);
		stream.writeAttribute("datetime", pi->datetime.toString("yyyy:MM:dd HH:mm:ss"));
		stream.writeAttribute("file", pi->filename);
		stream.writeAttribute("path", pi->filepath);
		stream.writeAttribute("sha", pi->sha1);
		stream.writeEndElement();
	}
	stream.writeEndElement();
	stream.writeEndDocument();
}

int PicStorage::size() const
{
	return mStorage.size();
}

PicInfo *PicStorage::info(int index) const
{
	return mStorage.values().at(index);
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
	int i = 0;
	while (i < res.size()) {
		if (int(res.at(i)) == 0)
			res.remove(i, 1);
		else
			++i;
	}
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
		QString value = fields.join(" ").trimmed();
		if (name == "Exif.Photo.DateTimeOriginal") {
			pi->datetime = QDateTime::fromString(value, "yyyy:MM:dd HH:mm:ss");
		}
		if (name == "Exif.Image.Model") {
			pi->camera = pi->camera + (pi->camera.isEmpty()?"":" ") + value;
		}
		if (name == "Exif.Image.Make") {
			pi->camera = value + (pi->camera.isEmpty()?"":" ") + pi->camera;
		}
	}

	return pi;
}

QStringList PicStorage::scanDir(QString path, QStringList *subPaths)
{
	QDir dir(path);
	QStringList filters;
	filters << "*.jpg" << "*.jpeg" << "*.png";
	QStringList res;
	QStringList files = dir.entryList(filters, QDir::Files);
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (QString fn, files) res += path + "/" + fn;
	foreach (QString d, dirs) res += scanDir(path + "/" + d, subPaths);
	if (subPaths && files.size() == dir.entryList(QDir::Files).size())
		subPaths->append(path);
	return res;
}

QString PicStorage::location() const
{
	return mLocation;
}

QString PicInfo::toString() const
{
	return QString("%1/%2:\t%3 %4 %5").
			arg(filepath).arg(filename).arg(sha1).
			arg(datetime.toString("yyyy-MM-dd")).arg(camera);
}
