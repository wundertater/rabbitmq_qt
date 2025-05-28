#include "Logger.h"

#include <QDebug>
#include <QTime>

std::unique_ptr<Logger> Logger::s_instance;

Logger::Logger(const QString &fileName, QtMsgType level)
{
  m_minLogLevel = level;
  m_logFile = std::make_unique<QFile>(fileName);
  if (m_logFile->open(QIODevice::Append | QIODevice::Text))
  {
    qInstallMessageHandler(&Logger::messageHandler);
    QString pattern = "["
                      "%{if-debug}D%{endif}"
                      "%{if-info}I%{endif}"
                      "%{if-warning}W%{endif}"
                      "%{if-critical}C%{endif}"
                      "%{if-fatal}F%{endif}"
                      "%{time ddMMyyyy hh:mm:ss.zzz }"
                      "%{appname}:%{threadid}:"
                      "%{file}:%{line}] %{message}\n";
    qSetMessagePattern(pattern);

    QTextStream out(m_logFile.get());
    QString logFileCreated = QString("Log started at: %1\n").arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz"));
    out << "\n" << logFileCreated;
    out << "Log line format: [DIWCF]ddMMyyyy hh:mm:ss.zzz appname:threadid:file:line] msg\n";
    m_logFile->flush();
   }
}

Logger::~Logger()
{
  qInstallMessageHandler(nullptr);
  m_logFile->close();
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  Logger *logger = s_instance.get();
  if (!logger->m_logFile)
    return;
  if (logger->m_minLogLevel == QtFatalMsg && type != QtFatalMsg)
    return;
  if (logger->m_minLogLevel == QtCriticalMsg && (type > QtFatalMsg || type < QtCriticalMsg))
    return;
  if (logger->m_minLogLevel == QtWarningMsg && (type > QtFatalMsg || type < QtWarningMsg))
    return;
  if (logger->m_minLogLevel == QtInfoMsg && type == QtDebugMsg)
    return;

  QTextStream out(logger->m_logFile.get());
  out << qFormatLogMessage(type, context, msg);
  logger->m_logFile->flush();
}

void Logger::setupLogging(const QString &filePath, QtMsgType level)
{
  Logger::s_instance = std::make_unique<Logger>(filePath, level);
}
