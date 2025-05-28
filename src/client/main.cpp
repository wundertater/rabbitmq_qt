#include "MainWindow.h"

#include <QApplication>
#include <QFileDialog>

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  QString configFileName = QFileDialog::getOpenFileName(nullptr, "Open Config File", "", "Config Files (*.ini)");
  if (configFileName.isEmpty())
    configFileName = "config.ini";

  MainWindow window(configFileName);
  window.show();
  return app.exec();
}
