#include "Server.h"

#include "protocol/Messages.pb.h"

#include <QDebug>

#include <stdexcept>

Server::Server(std::shared_ptr<IRabbitmqConnection> connection, const std::string& host, int port,
               const std::string& login, const std::string& password,
               int heartbeat, const std::string& vhost,
               const std::string& exchangeName,
               const std::string& responseQueueName, const std::string& requestQueueName)
  : m_connection(connection)
{
  m_socket = m_connection->openSocket(host, port);
  m_connection->login(login, password, heartbeat, vhost);
  m_channel = m_connection->openChannel();

  m_exchange = m_connection->declareExchange(*m_channel, exchangeName, "direct");
  m_responseQueue = m_connection->declareQueue(*m_channel, responseQueueName);
  m_requestQueue = m_connection->declareQueue(*m_channel, requestQueueName);

  m_responseBinding = m_connection->bind(*m_channel, *m_responseQueue, *m_exchange, responseQueueName);
  m_requestBinding = m_connection->bind(*m_channel, *m_requestQueue, *m_exchange, requestQueueName);

  const bool noAsk = true;
  const bool exclusive = true;
  m_connection->basicConsume(*m_channel, *m_requestQueue, noAsk, exclusive);
}

void Server::processRequestResponseCycle(std::chrono::milliseconds timeoutMillis)
{
  auto envelope = m_connection->timedConsumeMessage(timeoutMillis);
  if (!envelope)
    return;

  TestTask::Messages::Request request;
  if (!request.ParseFromString(envelope->getMessage()))
  {
    std::string errorMsg = "Server error: Failed to parse request message";
    qCritical() << QString::fromStdString(errorMsg);
    throw std::runtime_error(errorMsg);
  }
  else
    qInfo() << "Server received request with ID:" << QString::fromStdString(request.id()) << "and value:" << request.req();

  TestTask::Messages::Response response;
  response.set_id(request.id());
  response.set_res(generateResponseValue(request.req()));

  std::string responseStr;
  if (!response.SerializeToString(&responseStr))
  {
    std::string errorMsg = "Server error: Failed to serialize response message";
    qCritical() << QString::fromStdString(errorMsg);
    throw std::runtime_error(errorMsg);
  }
  else
    qInfo() << "Server prepared response for request ID:" << QString::fromStdString(response.id()) << "with result:" << response.res();

  m_connection->publishMessage(*m_channel, *m_exchange, *m_responseBinding, responseStr);
  qInfo() << "Server successfully published response for request ID:" << QString::fromStdString(request.id());
}

int Server::generateResponseValue(int reqValue)
{
  return reqValue * 2;
}
