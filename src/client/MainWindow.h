#ifndef CLIENT_MAINWINDOW__H
#define CLIENT_MAINWINDOW__H

#include "MessageReceiverThread.h"
#include "Client.h"

#include "ConfigManager/ConfigManager.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(const QString& configFile, QWidget *parent = nullptr);

public slots:
  void updateResponseField(const QString &response);
  void showError(const QString &error);

private slots:
  void onSendRequest();
  void onEditConfig();

private:
  void deleteMessageReceiverThread();


  QLineEdit *m_inputField = nullptr;
  QTextEdit *m_responseField = nullptr;
  QPushButton *m_sendButton = nullptr;
  QPushButton *m_configButton = nullptr;
  std::shared_ptr<ConfigManager> m_configManager;
  std::shared_ptr<Client> m_client;
  std::unique_ptr<MessageReceiverThread> m_receiver;
};
#endif
