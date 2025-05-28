#ifndef CLIENT_H
#define CLIENT_H

#include "RabbitMQClient/IRabbitmqConnection.h"
#include "RabbitMQClient/rabbitmqEntities.h"

#include <QUuid>

class Client
{
public:
  Client(std::shared_ptr<IRabbitmqConnection> connection,
         const std::string& host, int port,
         const std::string& login, const std::string& password,
         int heartbeat, const std::string& vhost,
         const std::string& exchangeName,
         const std::string& responseQueueName, const std::string& requestQueueName);
  ~Client() = default;

  QUuid getId() const {return m_id;}

  void sendRequest(int req);
  std::pair<bool, int> getResponse(std::chrono::milliseconds timeoutMillis);
private:
  std::shared_ptr<IRabbitmqConnection> m_connection;
  std::unique_ptr<RabbitmqSocket> m_socket;
  std::unique_ptr<RabbitmqChannel> m_channel;

  std::unique_ptr<RabbitmqExchange> m_exchange;
  std::unique_ptr<RabbitmqQueue> m_responseQueue;
  std::unique_ptr<RabbitmqQueue> m_requestQueue;
  std::unique_ptr<RabbitmqBind> m_responseBinding;
  std::unique_ptr<RabbitmqBind> m_requestBinding;

  QUuid m_id;
};

#endif
