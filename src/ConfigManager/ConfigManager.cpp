#include "ConfigManager.h"

QtMsgType ConfigManager::getLogLevel() const
{
  QString level = m_settings.value("Logging/Level", "info").toString();
  return QStringToQtMsgType(level);
}

void ConfigManager::setLogLevel(QtMsgType logLevel)
{
  QString res = QtMsgTypeToQString(logLevel);
  m_settings.setValue("Logging/Level", res);
}

QString ConfigManager::QtMsgTypeToQString(QtMsgType type)
{
  switch (type)
  {
    case QtDebugMsg:
      return "debug";
    case QtWarningMsg:
      return "warning";
    case QtCriticalMsg:
      return "critical";
    case QtFatalMsg:
      return "fatal";
    case QtInfoMsg:
      return "info";
    default:
      throw std::invalid_argument("Invalid QtMsgType: " + std::to_string(static_cast<int>(type)));
  }
}

QtMsgType ConfigManager::QStringToQtMsgType(const QString &string)
{
  QString level = string.toLower();
  if (level == "debug")
    return QtDebugMsg;
  else if (level == "warning")
    return QtWarningMsg;
  else if (level == "critical")
    return QtCriticalMsg;
  else if (level == "fatal")
    return QtFatalMsg;
  else if (level == "info")
    return QtInfoMsg;
  throw std::invalid_argument("Invalid MsgType QString: " + string.toStdString());
}
