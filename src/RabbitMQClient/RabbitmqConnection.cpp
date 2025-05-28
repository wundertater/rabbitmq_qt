#include "RabbitmqConnection.h"
#include "rabbitmqEntities.h"
#include "validation.h"

#include <QDebug>

RabbitmqConnection::RabbitmqConnection(Private)
  :m_connection(amqp_new_connection())
{
  if (!m_connection)
  {
    std::string msg("Failed to create AMQP connection");
    qCritical() << QString::fromStdString(msg);
    throw std::runtime_error(msg);
  }
  else
    qInfo() << "Connection is open";
}

RabbitmqConnection::~RabbitmqConnection()
{
  amqp_rpc_reply_t repl = amqp_connection_close(m_connection, AMQP_REPLY_SUCCESS);
  std::string msg = validation(repl, "Error closing connection");
  if (msg.empty())
    qInfo() << "Connection is closed";

  int status = amqp_destroy_connection(m_connection);
  if (status != AMQP_STATUS_OK)
    qCritical() << "Error destroying connection: " << amqp_error_string2(status);
  else
    qInfo() << "Connection is destroyed";
}

std::shared_ptr<RabbitmqConnection> RabbitmqConnection::create()
{
  return std::make_shared<RabbitmqConnection>(Private());
}

std::shared_ptr<IRabbitmqConnection> RabbitmqConnection::share()
{
  return shared_from_this();
}

std::unique_ptr<RabbitmqSocket> RabbitmqConnection::openSocket(const std::string &host, int port)
{
  return std::make_unique<RabbitmqSocket>(m_connection, host, port);
}

void RabbitmqConnection::login(const std::string &login, const std::string &password,
                               int heartbeatInSeconds, const std::string& vhost)
{
  auto repl = amqp_login(m_connection, vhost.c_str(), AMQP_DEFAULT_MAX_CHANNELS,
                         AMQP_DEFAULT_FRAME_SIZE, heartbeatInSeconds,
                         AMQP_SASL_METHOD_PLAIN, login.c_str(), password.c_str());
  std::string msg = validation(repl, "Error login for user: " + login);
  if (!msg.empty())
    throw std::runtime_error(msg);
  else
    qInfo() << "Login has occurred for user: " << QString::fromStdString(login);
}

std::unique_ptr<RabbitmqChannel> RabbitmqConnection::openChannel()
{
  auto res = std::make_unique<RabbitmqChannel>(share(), m_connection, m_freeChannelId);
  ++m_freeChannelId;
  return res;
}

std::unique_ptr<RabbitmqExchange> RabbitmqConnection::declareExchange(const RabbitmqChannel& channel,
                                                                      const std::string& exchangeName,
                                                                      const std::string& exchangeType)
{
  return std::make_unique<RabbitmqExchange>(share(), m_connection, channel.getId(), exchangeName, exchangeType);
}

std::unique_ptr<RabbitmqQueue> RabbitmqConnection::declareQueue(const RabbitmqChannel &channel, const std::string &queueName)
{
  return std::make_unique<RabbitmqQueue>(share(), m_connection, channel.getId(), queueName);
}

std::unique_ptr<RabbitmqBind> RabbitmqConnection::bind(const RabbitmqChannel &channel, const RabbitmqQueue &queue,
                                                       const RabbitmqExchange &exchange, const std::string &bindingKey)
{
  return std::make_unique<RabbitmqBind>(share(), m_connection, channel.getId(), queue.getName(),
                                        exchange.getName(), bindingKey);
}

void RabbitmqConnection::basicConsume(const RabbitmqChannel &channel, const RabbitmqQueue &queue, bool noAsk, bool exclusive)
{
  qInfo() << "Preparing to consume from queue:" << QString::fromStdString(queue.getName())
          << "on channel:" << channel.getId()
          << "with noAsk:" << noAsk
          << "and exclusive:" << exclusive;

  const amqp_bytes_t emptyTag = amqp_empty_bytes;
  const amqp_table_t emptyArgs = amqp_empty_table;
  const bool noLocal = true; // запрещает клиенту получать сообщения, которые он отправил сам
  amqp_basic_consume(m_connection, channel.getId(),
                     amqp_cstring_bytes(queue.getName().c_str()),
                     emptyTag, noLocal, noAsk, exclusive, emptyArgs);
  auto repl = amqp_get_rpc_reply(m_connection);
  std::string msg = validation(repl, "Error consuming from queue: " + queue.getName());
  if (!msg.empty())
  {
    throw std::runtime_error(msg);
  }
  else
    qInfo() << "Successfully started consuming from queue: " << QString::fromStdString(queue.getName());
}

void RabbitmqConnection::publishMessage(const RabbitmqChannel& channel, const RabbitmqExchange& exchange,
                                        const RabbitmqBind& binding, std::string message)
{
  const bool mandatory = true;
  const bool immediate = false;

  amqp_bytes_t bytes;
  bytes.bytes = const_cast<char*>(message.c_str());
  bytes.len = message.size();

  int status = amqp_basic_publish(m_connection, channel.getId(),
                     amqp_cstring_bytes(exchange.getName().c_str()),
                     amqp_cstring_bytes(binding.getBindingKey().c_str()),
                     mandatory, immediate, nullptr,
                     bytes);
  if (status != AMQP_STATUS_OK)
  {
    std::string errorMsg = "Error publish message: ";
    errorMsg += amqp_error_string2(status);
    qCritical() << QString::fromStdString(errorMsg);
    throw std::runtime_error(errorMsg);
  }
  else
    qInfo() << "Successfully publish message:" << QString::fromStdString(message);
}

void RabbitmqConnection::ack(const IRabbitmqEnvelope& envelope)
{
  const bool multipleAsk = false;
  int status = amqp_basic_ack(m_connection, envelope.getChannel(), envelope.getDeliveryTag(), multipleAsk);
  if (status != 0)
  {
    std::string errorMsg = "Failed to ack";
    qCritical() << QString::fromStdString(errorMsg);
    throw std::runtime_error(errorMsg);
  }
  else
    qInfo() << "Successfully ack";
}

void RabbitmqConnection::reject(const IRabbitmqEnvelope &envelope)
{
  const bool requeue = true;
  int status = amqp_basic_reject(m_connection, envelope.getChannel(), envelope.getDeliveryTag(), requeue);
  if (status != 0)
  {
    std::string errorMsg = "Failed to reject";
    qCritical() << QString::fromStdString(errorMsg);
    throw std::runtime_error(errorMsg);
  }
    qInfo() << "Successfully reject";
}

std::unique_ptr<IRabbitmqEnvelope> RabbitmqConnection::consumeMessage()
{
  return consumeMessageInternal(nullptr);
}

std::unique_ptr<IRabbitmqEnvelope> RabbitmqConnection::timedConsumeMessage(std::chrono::milliseconds timeoutMillis)
{
  std::chrono::seconds seconds = std::chrono::duration_cast<std::chrono::seconds>(timeoutMillis);
  std::chrono::microseconds microseconds = timeoutMillis - seconds;

  struct timeval timeout;
  timeout.tv_sec = seconds.count();
  timeout.tv_usec = microseconds.count();

  return consumeMessageInternal(&timeout);
}

std::unique_ptr<IRabbitmqEnvelope> RabbitmqConnection::consumeMessageInternal(struct timeval* timeout)
{
  auto envelope = std::make_unique<RabbitmqEnvelope>();
  amqp_maybe_release_buffers(m_connection);

  int unusedFlag = 0;
  auto repl = amqp_consume_message(m_connection, envelope->get(), timeout, unusedFlag);

  if (repl.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION && repl.library_error == AMQP_STATUS_TIMEOUT)
  {
    qInfo() << "Timeout occurred while waiting for a message.";
    return nullptr;
  }

  std::string errorMsg = validationAfterConsumeMessage(m_connection, repl, "Failed consume message");
  if (!errorMsg.empty())
    throw std::runtime_error(errorMsg);
  else
    qInfo() << "Successfully consumed message:" << QString::fromStdString(envelope->getMessage());

  return envelope;
}
