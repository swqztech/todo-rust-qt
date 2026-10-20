#include "main_window.h"

#include "todo_ffi.h"

#include <QApplication>
#include <QDir>
#include <QStandardPaths>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  QApplication::setApplicationName(QStringLiteral("todo-rust-qt"));

  const QString dataDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(dataDir);
  todo_init(dataDir.toUtf8().constData());

  MainWindow w;
  w.show();
  return app.exec();
}
