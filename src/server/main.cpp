#include "Server.h"

#include "Logger/Logger.h"
#include "ConfigManager/ConfigManager.h"
#include "RabbitMQClient/RabbitmqConnection.h"

#include <QDebug>
#include <QCoreApplication>
#include <QCommandLineParser>

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);
  QCommandLineParser parser;
  parser.setApplicationDescription("Server");
  parser.addHelpOption();

  QCommandLineOption configOption(QStringList() << "c" << "config", "Specify the config file.", "configFile", "config.ini");
  parser.addOption(configOption);

  parser.process(app);
  QString configFileName = parser.value(configOption);

  ConfigManager config(configFileName);
  if (config.isLoggingEnabled())
    Logger::setupLogging(config.getLogFilePath(), config.getLogLevel());
  qInfo() << "LOGGER START";

  Server s(RabbitmqConnection::create(),
           config.getHost().toStdString(), config.getPort(),
           config.getLogin().toStdString(), config.getPassword().toStdString(),
           config.getHeartbeat(), config.getVhost().toStdString(),
           config.getExchangeName().toStdString(),
           config.getResponseQueueName().toStdString(),
           config.getRequestQueueName().toStdString());
  while (true)
    s.processRequestResponseCycle(std::chrono::milliseconds(100));
}
