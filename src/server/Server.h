#ifndef SERVER_H
#define SERVER_H

#include "RabbitMQClient/IRabbitmqConnection.h"
#include "RabbitMQClient/rabbitmqEntities.h"

class Server
{
public:
  Server(std::shared_ptr<IRabbitmqConnection> connection,
         const std::string& host, int port,
         const std::string& login, const std::string& password,
         int heartbeat, const std::string& vhost,
         const std::string& exchangeName,
         const std::string& responseQueueName, const std::string& requestQueueName);
  ~Server() = default;

  void processRequestResponseCycle(std::chrono::milliseconds timeoutMillis);
  static int generateResponseValue(int reqValue);
private:
  std::shared_ptr<IRabbitmqConnection> m_connection;
  std::unique_ptr<RabbitmqSocket> m_socket;
  std::unique_ptr<RabbitmqChannel> m_channel;

  std::unique_ptr<RabbitmqExchange> m_exchange;
  std::unique_ptr<RabbitmqQueue> m_responseQueue;
  std::unique_ptr<RabbitmqQueue> m_requestQueue;
  std::unique_ptr<RabbitmqBind> m_responseBinding;
  std::unique_ptr<RabbitmqBind> m_requestBinding;
};

#endif
