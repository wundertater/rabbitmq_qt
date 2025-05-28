#ifndef LOGGER_H
#define LOGGER_H

#include <QString>
#include <QFile>

#include <memory>

class Logger {
public:
  Logger(const QString &filePath, QtMsgType level);
  ~Logger();

  static void setupLogging(const QString &filePath, QtMsgType level);

  static std::unique_ptr<Logger> s_instance;
private:
  static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

  QtMsgType m_minLogLevel;
  std::unique_ptr<QFile> m_logFile;
};

#endif
