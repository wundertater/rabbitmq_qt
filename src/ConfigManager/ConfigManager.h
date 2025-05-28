#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QSettings>
#include <QString>

class ConfigManager
{
public:
  ConfigManager(const QString& fileName) : m_settings(fileName, QSettings::IniFormat) {}

  QString getHost() const { return m_settings.value("Connection/Host", "localhost").toString(); }
  void setHost(const QString& host) { m_settings.setValue("Connection/Host", host); }

  int getPort() const { return m_settings.value("Connection/Port", 5672).toInt(); }
  void setPort(int port) { m_settings.setValue("Connection/Port", port); }

  QString getLogin() const { return m_settings.value("Connection/Login", "guest").toString(); }
  void setLogin(const QString& login) { m_settings.setValue("Connection/Login", login); }

  QString getPassword() const { return m_settings.value("Connection/Password", "guest").toString(); }
  void setPassword(const QString& password) { m_settings.setValue("Connection/Password", password); }

  int getHeartbeat() const { return m_settings.value("Connection/Heartbeat", 0).toInt(); }
  void setHeartbeat(int heartbeat) { m_settings.setValue("Connection/Heartbeat", heartbeat); }

  QString getVhost() const { return m_settings.value("Connection/Vhost", "/").toString(); }
  void setVhost(const QString& vhost) { m_settings.setValue("Connection/Vhost", vhost); }

  QString getExchangeName() const { return m_settings.value("Messaging/ExchangeName", "defaultExchange").toString(); }
  void setExchangeName(const QString& exchangeName) { m_settings.setValue("Messaging/ExchangeName", exchangeName); }

  QString getRequestQueueName() const { return m_settings.value("Messaging/RequestQueueName", "defaultRequestQueue").toString(); }
  void setRequestQueueName(const QString& requestQueueName) { m_settings.setValue("Messaging/RequestQueueName", requestQueueName); }

  QString getResponseQueueName() const { return m_settings.value("Messaging/ResponseQueueName", "defaultResponseQueue").toString(); }
  void setResponseQueueName(const QString& responseQueueName) { m_settings.setValue("Messaging/ResponseQueueName", responseQueueName); }

  bool isLoggingEnabled() const { return m_settings.value("Logging/Enabled", true).toBool(); }
  void setLoggingEnabled(bool enabled) { m_settings.setValue("Logging/Enabled", enabled); }

  QString getLogFilePath() const { return m_settings.value("Logging/FilePath", "logs.txt").toString(); }
  void setLogFilePath(const QString& logFilePath) { m_settings.setValue("Logging/FilePath", logFilePath); }

  QtMsgType getLogLevel() const;

  void setLogLevel(QtMsgType logLevel);

  static QString QtMsgTypeToQString(QtMsgType type);
  static QtMsgType QStringToQtMsgType(const QString& string);

private:
  QSettings m_settings;
};

#endif
