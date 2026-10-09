// Compiles QML files with the same import path as qutim and reports errors.
// Usage: qmlcheck <file.qml>...   (exit code = number of broken files)
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlComponent>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	QQmlEngine engine;
	const QString imports = QDir(QCoreApplication::applicationDirPath()
	                             + QStringLiteral("/../share/apps/qutim/imports")).absolutePath();
	engine.addImportPath(imports);
	QTextStream out(stdout);
	int failed = 0;
	for (const QString &path : app.arguments().mid(1)) {
		QQmlComponent component(&engine, QUrl::fromLocalFile(QFileInfo(path).absoluteFilePath()));
		if (component.isError()) {
			++failed;
			out << "FAIL " << path << "\n";
			for (const QQmlError &error : component.errors())
				out << "    " << error.toString() << "\n";
		} else {
			out << "ok   " << path << "\n";
		}
	}
	return failed;
}
