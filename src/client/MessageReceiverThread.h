#ifndef CLIENT_MESSAGERECEIVERTHREAD_H
#define CLIENT_MESSAGERECEIVERTHREAD_H

#include "Client.h"

#include <QThread>
#include <QDebug>

class MessageReceiverThread : public QThread
{
  Q_OBJECT

public:
  MessageReceiverThread(std::shared_ptr<Client> client, std::chrono::milliseconds timeout, QObject *parent = nullptr)
    : QThread(parent), m_client(client), m_timeout(timeout)
  {
    if (!m_client)
      throw std::invalid_argument("nullptr client");
  }

  void stop() { m_running = false; }

  void run() override
  {
    m_running = true;
    while (m_running)
    {
      try
      {
        auto result = m_client->getResponse(m_timeout);
        bool success = result.first;
        int responseValue = result.second;
        if (success)
          emit responseReceived(QString::number(responseValue));
      }
      catch (const std::exception& e)
      {
        qCritical() << "Error in MessageReceiverThread:" << e.what();
        emit errorOccurred(QString::fromStdString(e.what()));
        break;
      }
      catch (...)
      {
        qCritical() << "Unknown error in MessageReceiverThread.";
        emit errorOccurred("Unknown error occurred.");
        break;
      }
    }
  }

signals:
  void responseReceived(const QString &response);
  void errorOccurred(const QString &error);

private:
  std::shared_ptr<Client> m_client;
  bool m_running = false;
  const std::chrono::milliseconds m_timeout;
};

#endif // CLIENT_MESSAGERECEIVERTHREAD_H
