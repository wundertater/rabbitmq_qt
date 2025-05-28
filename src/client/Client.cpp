#include "Client.h"

#include "protocol/Messages.pb.h"

#include <QDebug>

#include <stdexcept>

Client::Client(std::shared_ptr<IRabbitmqConnection> connection, const std::string& host, int port,
               const std::string& login, const std::string& password,
               int heartbeat, const std::string &vhost,
               const std::string& exchangeName,
               const std::string& responseQueueName, const std::string& requestQueueName)
  : m_connection(connection)
{
  m_id = QUuid::createUuid();
  m_socket = m_connection->openSocket(host, port);
  m_connection->login(login, password, heartbeat, vhost);
  m_channel = m_connection->openChannel();

  m_exchange = m_connection->declareExchange(*m_channel, exchangeName, "direct");
  m_responseQueue = m_connection->declareQueue(*m_channel, responseQueueName);
  m_requestQueue = m_connection->declareQueue(*m_channel, requestQueueName);

  m_responseBinding = m_connection->bind(*m_channel, *m_responseQueue, *m_exchange, responseQueueName);
  m_requestBinding = m_connection->bind(*m_channel, *m_requestQueue, *m_exchange, requestQueueName);

  const bool noAsk = false;
  const bool exclusive = false;
  m_connection->basicConsume(*m_channel, *m_responseQueue, noAsk, exclusive);
}

void Client::sendRequest(int req)
{
  TestTask::Messages::Request request;
  request.set_id(m_id.toString().toStdString());
  request.set_req(req);

  std::string requestStr;
  if (!request.SerializeToString(&requestStr))
  {
    std::string errorMsg = "Client error: Failed to serialize request message";
    qCritical() << QString::fromStdString(errorMsg);
    throw std::runtime_error(errorMsg);
  }
  else
    qInfo() << "Client sending request with ID:" << QString::fromStdString(request.id()) << "and value:" << request.req();

  m_connection->publishMessage(*m_channel, *m_exchange, *m_requestBinding, requestStr);
  qInfo() << "Client request with ID:" << QString::fromStdString(request.id()) << "successfully published.";
}

std::pair<bool, int> Client::getResponse(std::chrono::milliseconds timeoutMillis)
{
  auto envelope = m_connection->timedConsumeMessage(timeoutMillis);
  if (!envelope)
    return {false, 0};

  TestTask::Messages::Response response;
  if (!response.ParseFromString(envelope->getMessage()))
  {
    std::string errorMsg = "Client error: Failed to parse response message";
    qCritical() << QString::fromStdString(errorMsg);
    throw std::runtime_error(errorMsg);
  }
  else
    qInfo() << "Client received response for ID:" << QString::fromStdString(response.id()) << "with result:" << response.res();

  if (response.id() == m_id.toString().toStdString())
  {
    m_connection->ack(*envelope);
    qInfo() << "Client acknowledged response for request ID:" << QString::fromStdString(response.id());
    return {true, response.res()};
  }
  else
  {
    m_connection->reject(*envelope);
    qInfo() << "Client rejected response for request ID:" << QString::fromStdString(response.id());
  }
  return {false, 0};
}
