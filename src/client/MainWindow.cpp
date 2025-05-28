#include "MainWindow.h"
#include "ConfigDialog.h"

#include "RabbitMQClient/RabbitmqConnection.h"
#include "Logger/Logger.h"

#include <QMessageBox>
#include <QVBoxLayout>

MainWindow::MainWindow(const QString &configFile, QWidget *parent)
  : QMainWindow(parent), m_configManager(std::make_shared<ConfigManager>(configFile))
{
  if (m_configManager->isLoggingEnabled())
    Logger::setupLogging(m_configManager->getLogFilePath(), m_configManager->getLogLevel());

  setWindowTitle("RabbitMQ Client");

  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *layout = new QVBoxLayout(centralWidget);

  m_inputField = new QLineEdit(this);
  m_inputField->setPlaceholderText("Введите число...");
  layout->addWidget(m_inputField);

  m_sendButton = new QPushButton("Отправить запрос", this);
  layout->addWidget(m_sendButton);

  m_responseField = new QTextEdit(this);
  m_responseField->setReadOnly(true);
  layout->addWidget(m_responseField);

  m_configButton = new QPushButton("Настройки", this);
  layout->addWidget(m_configButton);

  connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendRequest);
  connect(m_configButton, &QPushButton::clicked, this, &MainWindow::onEditConfig);
}

void MainWindow::onSendRequest()
{
  bool ok;
  int requestValue = m_inputField->text().toInt(&ok);
  if (!ok)
  {
    qWarning() << "Input error: enter the correct number";
    QMessageBox::warning(this, "Ошибка", "Требуется ввести число");
    return;
  }

  try
  {
    if (!m_client)
      m_client = std::make_shared<Client>(RabbitmqConnection::create(),
                                          m_configManager->getHost().toStdString(), m_configManager->getPort(),
                                          m_configManager->getLogin().toStdString(), m_configManager->getPassword().toStdString(),
                                          m_configManager->getHeartbeat(), m_configManager->getVhost().toStdString(),
                                          m_configManager->getExchangeName().toStdString(),
                                          m_configManager->getResponseQueueName().toStdString(),
                                          m_configManager->getRequestQueueName().toStdString());

    m_client->sendRequest(requestValue);

    if (m_receiver)
      deleteMessageReceiverThread();

    std::chrono::milliseconds timeout(500);
    m_receiver = std::make_unique<MessageReceiverThread>(m_client, timeout, this);
    connect(m_receiver.get(), &MessageReceiverThread::responseReceived, this, &MainWindow::updateResponseField);
    connect(m_receiver.get(), &MessageReceiverThread::errorOccurred, this, &MainWindow::showError);
    m_receiver->start();
  } catch (const std::exception& e)
  {
    qCritical() << "Error in onSendRequest: " + QString::fromStdString(e.what());
    QMessageBox::critical(this, "Ошибка", e.what());
    return;
  }
}

void MainWindow::onEditConfig()
{
  if (m_receiver)
    deleteMessageReceiverThread();

  ConfigDialog configDialog(m_configManager, this);
  if (configDialog.exec() == QDialog::Accepted)
  {
    if (configDialog.connectionSettingsChanged())
    {
      m_client = nullptr;
    }
  }
}

void MainWindow::deleteMessageReceiverThread()
{
  m_receiver->stop();
  m_receiver->wait();
  disconnect(m_receiver.get(), &MessageReceiverThread::responseReceived, this, &MainWindow::updateResponseField);
  disconnect(m_receiver.get(), &MessageReceiverThread::errorOccurred, this, &MainWindow::showError);
  m_receiver = nullptr;
}

void MainWindow::updateResponseField(const QString &response)
{
  m_responseField->setText("Ответ: " + response);
}

void MainWindow::showError(const QString& error)
{
  QMessageBox::critical(this, "Ошибка", error);
}

