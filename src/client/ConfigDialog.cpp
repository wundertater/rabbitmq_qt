#include "ConfigDialog.h"

#include <QPushButton>
#include <QLabel>
#include <QGridLayout>

ConfigDialog::ConfigDialog(std::shared_ptr<ConfigManager> configManager, QWidget *parent)
  : QDialog(parent), m_configManager(configManager)
{
  setWindowTitle("Настройки клиента");

  QGridLayout *layout = new QGridLayout(this);

  m_hostEdit = new QLineEdit(m_configManager->getHost(), this);
  m_portEdit = new QLineEdit(QString::number(m_configManager->getPort()), this);
  m_loginEdit = new QLineEdit(m_configManager->getLogin(), this);
  m_passwordEdit = new QLineEdit(m_configManager->getPassword(), this);
  m_exchangeEdit = new QLineEdit(m_configManager->getExchangeName(), this);
  m_vhostEdit = new QLineEdit(m_configManager->getVhost(), this);
  m_requestQueueEdit = new QLineEdit(m_configManager->getRequestQueueName(), this);
  m_responseQueueEdit = new QLineEdit(m_configManager->getResponseQueueName(), this);
  m_heartbeatEdit = new QLineEdit(QString::number(m_configManager->getHeartbeat()), this);

  m_loggingEnabledCheckbox = new QCheckBox("Включить логирование", this);
  m_loggingEnabledCheckbox->setChecked(m_configManager->isLoggingEnabled());

  m_logFilePathEdit = new QLineEdit(m_configManager->getLogFilePath(), this);
  m_logLevelEdit = new QLineEdit(ConfigManager::QtMsgTypeToQString(m_configManager->getLogLevel()), this);

  layout->addWidget(new QLabel("Хост:"), 0, 0);
  layout->addWidget(m_hostEdit, 0, 1);
  layout->addWidget(new QLabel("Порт:"), 1, 0);
  layout->addWidget(m_portEdit, 1, 1);
  layout->addWidget(new QLabel("Логин:"), 2, 0);
  layout->addWidget(m_loginEdit, 2, 1);
  layout->addWidget(new QLabel("Пароль:"), 3, 0);
  layout->addWidget(m_passwordEdit, 3, 1);
  layout->addWidget(new QLabel("Имя обменника:"), 4, 0);
  layout->addWidget(m_exchangeEdit, 4, 1);
  layout->addWidget(new QLabel("VHost:"), 5, 0);
  layout->addWidget(m_vhostEdit, 5, 1);
  layout->addWidget(new QLabel("Имя очереди запросов:"), 6, 0);
  layout->addWidget(m_requestQueueEdit, 6, 1);
  layout->addWidget(new QLabel("Имя очереди ответов:"), 7, 0);
  layout->addWidget(m_responseQueueEdit, 7, 1);
  layout->addWidget(new QLabel("Heartbeat:"), 8, 0);
  layout->addWidget(m_heartbeatEdit, 8, 1);
  layout->addWidget(m_loggingEnabledCheckbox, 9, 0, 1, 2);
  layout->addWidget(new QLabel("Путь к лог-файлу:"), 10, 0);
  layout->addWidget(m_logFilePathEdit, 10, 1);
  layout->addWidget(new QLabel("Уровень логирования:"), 11, 0);
  layout->addWidget(m_logLevelEdit, 11, 1);

  QPushButton *saveButton = new QPushButton("Сохранить", this);
  layout->addWidget(saveButton, 12, 0, 1, 2);

  connect(saveButton, &QPushButton::clicked, this, &ConfigDialog::onSave);
}

void ConfigDialog::onSave() {
  auto oldHost = m_configManager->getHost();
  auto oldPort = m_configManager->getPort();
  auto oldLogin = m_configManager->getLogin();
  auto oldPassword = m_configManager->getPassword();
  auto oldExchangeName = m_configManager->getExchangeName();
  auto oldVhost = m_configManager->getVhost();
  auto oldRequestQueueName = m_configManager->getRequestQueueName();
  auto oldResponseQueueName = m_configManager->getResponseQueueName();
  auto oldHeartbeat = m_configManager->getHeartbeat();


  m_configManager->setHost(m_hostEdit->text());
  m_configManager->setPort(m_portEdit->text().toInt());
  m_configManager->setLogin(m_loginEdit->text());
  m_configManager->setPassword(m_passwordEdit->text());
  m_configManager->setExchangeName(m_exchangeEdit->text());
  m_configManager->setVhost(m_vhostEdit->text());
  m_configManager->setRequestQueueName(m_requestQueueEdit->text());
  m_configManager->setResponseQueueName(m_responseQueueEdit->text());
  m_configManager->setHeartbeat(m_heartbeatEdit->text().toInt());
  m_configManager->setLoggingEnabled(m_loggingEnabledCheckbox->isChecked());
  m_configManager->setLogFilePath(m_logFilePathEdit->text());
  m_configManager->setLogLevel(ConfigManager::QStringToQtMsgType(m_logLevelEdit->text()));

  if (m_configManager->getHost() != oldHost ||
    m_configManager->getPort() != oldPort ||
    m_configManager->getLogin() != oldLogin ||
    m_configManager->getPassword() != oldPassword ||
    m_configManager->getExchangeName() != oldExchangeName ||
    m_configManager->getVhost() != oldVhost ||
    m_configManager->getRequestQueueName() != oldRequestQueueName ||
    m_configManager->getResponseQueueName() != oldResponseQueueName ||
    m_configManager->getHeartbeat() != oldHeartbeat)
  {
    m_connectionSettingsChanged = true;
  }
  accept();
}
