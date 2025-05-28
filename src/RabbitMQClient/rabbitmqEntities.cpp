#include "rabbitmqEntities.h"
#include "validation.h"

#include <QDebug>

RabbitmqSocket::RabbitmqSocket(amqp_connection_state_t connection, const std::string &host, int port)
  : m_socket(amqp_tcp_socket_new(connection))
{
  if (!m_socket)
  {
    std::string msg = "Failed to create TCP socket for host: " + host + " on port: " + std::to_string(port);
    qCritical() << QString::fromStdString(msg);
    throw std::runtime_error(msg);
  }
  else
  {
    qInfo() << "Creating TCP socket for host: " << QString::fromStdString(host) << " on port: " << port;
    int status = amqp_socket_open(m_socket, host.c_str(), port);
    if (status != AMQP_STATUS_OK)
    {
      std::string msg = "Failed to open TCP socket for host: " + host + " on port: " + std::to_string(port);
      msg += amqp_error_string2(status);
      qCritical() << QString::fromStdString(msg);
      throw std::runtime_error(msg);
    }
    else
      qInfo() << "TCP socket opened successfully for host: " << QString::fromStdString(host) << " on port: " << port;
  }
}

RabbitmqChannel::RabbitmqChannel(std::shared_ptr<IRabbitmqConnection> connection,
                amqp_connection_state_t amqpConnection, amqp_channel_t channel)
  : m_connection(connection), m_AmqpConnection(amqpConnection), m_channel(channel)
{
  amqp_channel_open(m_AmqpConnection, m_channel);
  auto repl = amqp_get_rpc_reply(m_AmqpConnection);
  std::string msg = validation(repl, "Error opening channel: " + std::to_string(m_channel));
  if (!msg.empty())
    throw std::runtime_error(msg);
  else
    qInfo() << "Channel opened successfully: " << m_channel;
}

RabbitmqChannel::~RabbitmqChannel()
{
  auto shared = m_connection.lock();
  if (shared)
  {
    auto repl = amqp_channel_close(m_AmqpConnection, m_channel, AMQP_REPLY_SUCCESS);
    std::string msg = validation(repl, "Error closing channel: " + std::to_string(m_channel));
    if (msg.empty())
      qInfo() << "Channel closed successfully: " << m_channel;
  }
  else
  {
    qCritical() << "Error closing channel" << m_channel << ": undefined connection";
  }
}

RabbitmqExchange::RabbitmqExchange(std::shared_ptr<IRabbitmqConnection> connection, amqp_connection_state_t amqpConnection,
                                   amqp_channel_t channel, const std::string& exchangeName, const std::string& exchangeType)
  : m_connection(connection), m_AmqpConnection(amqpConnection), m_channel(channel), m_ExchangeName(exchangeName)
{
  qInfo() << "Declaring exchange: " << QString::fromStdString(m_ExchangeName)
          << " of type: " << QString::fromStdString(exchangeType)
          << " on channel: " << m_channel;

  const bool existenceCheck = false; // при true будет проверка на существование очереди без её создания
  const bool durable = true; // при true обменник сохранится после перезагрузки брокера
  const bool autoDelete = true; // при true обменник будет удален, когда все очереди, связанные с ним отключатся
  const bool internal = false; // при true не будет доступа у других клиентов
  const amqp_table_t emptyArgs = amqp_empty_table;

  amqp_exchange_declare(m_AmqpConnection, m_channel,
                        amqp_cstring_bytes(m_ExchangeName.c_str()),
                        amqp_cstring_bytes(exchangeType.c_str()),
                        existenceCheck, durable, autoDelete, internal, emptyArgs);
  auto repl = amqp_get_rpc_reply(m_AmqpConnection);
  std::string msg = validation(repl, "Error declaring exchange: " + m_ExchangeName);
  if (!msg.empty())
    throw std::runtime_error(msg);
  else
    qInfo() << "Successfully declared exchange: " << QString::fromStdString(m_ExchangeName);
}

RabbitmqExchange::~RabbitmqExchange()
{
  /*
  qInfo() << "Deleting exchange:" << QString::fromStdString(m_ExchangeName) << "on channel:" << m_channel;

  auto shared = m_connection.lock();
  if (shared)
  {
    const bool onlyUnused = true; // Если установлено в true, обменник будет удален только в том случае, если он не используется
    amqp_exchange_delete(m_AmqpConnection, m_channel, amqp_cstring_bytes(m_ExchangeName.c_str()), onlyUnused);
    auto repl = amqp_get_rpc_reply(m_AmqpConnection);
    std::string msg = validation(repl, "Error deleting exchange: " + m_ExchangeName);
    if (msg.empty())
      qInfo() << "Successfully deleted exchange:" << QString::fromStdString(m_ExchangeName);
  }
  else
  {
    qCritical() << "Error deleting exchange: " << QString::fromStdString(m_ExchangeName) << ": undefined connection";
  }
  */
}

RabbitmqQueue::RabbitmqQueue(std::shared_ptr<IRabbitmqConnection> connection, amqp_connection_state_t amqpConnection,
                             amqp_channel_t channel, const std::string& queueName)
  : m_connection(connection), m_AmqpConnection(amqpConnection), m_channel(channel), m_QueueName(queueName)
{
  qInfo() << "Declaring queue: " << QString::fromStdString(m_QueueName) << " on channel: " << m_channel;

  const bool existenceCheck = false; // при true будет проверка на существование очереди без её создания
  const bool durable = true; // при true очередь сохранится после перезагрузки брокера
  const bool autoDelete = true; // при true очередь будет удалена, автоматически
  const bool exclusive = false; // при true не будет доступа для подключения у других клиентов
  const amqp_table_t emptyArgs = amqp_empty_table;

  amqp_queue_declare(m_AmqpConnection, m_channel,
                     amqp_cstring_bytes(m_QueueName.c_str()),
                     existenceCheck, durable, exclusive, autoDelete, emptyArgs);
  auto repl = amqp_get_rpc_reply(m_AmqpConnection);
  std::string msg = validation(repl, "Error declaring queue: " + m_QueueName);
  if (!msg.empty())
    throw std::runtime_error(msg);
  else
    qInfo() << "Successfully declared queue: " << QString::fromStdString(m_QueueName);
}

RabbitmqQueue::~RabbitmqQueue()
{
  /*
  auto shared = m_connection.lock();
  if (shared)
  {
    qInfo() << "Deleting queue: " << QString::fromStdString(m_QueueName) << "on channel: " << m_channel;

    const bool onlyUnused = true; // Если установлено в true, очередь будет удалена только в том случае, если она не используется
    const bool onlyEmpty = true; // Если установлено в true, очередь может быть удалена только в том случае, если она пуста
    amqp_queue_delete(m_AmqpConnection, m_channel, amqp_cstring_bytes(m_QueueName.c_str()), onlyUnused, onlyEmpty);
    auto repl = amqp_get_rpc_reply(m_AmqpConnection);
    std::string msg = validation(repl, "Error deleting queue: " + m_QueueName);
    if (msg.empty())
      qInfo() << "Successfully deleted queue:" << QString::fromStdString(m_QueueName);
  }
  else
  {
    qCritical() << "Error deleting queue: " << QString::fromStdString(m_QueueName) << ": undefined connection";
  }
  */
}

RabbitmqBind::RabbitmqBind(std::shared_ptr<IRabbitmqConnection> connection, amqp_connection_state_t amqpConnection,
                           amqp_channel_t channel, const std::string &queueName,
                           const std::string &exchangeName, const std::string &bindingKey)
  : m_connection(connection), m_AmqpConnection(amqpConnection), m_channel(channel), m_QueueName(queueName),
    m_ExchangeName(exchangeName), m_BindingKey(bindingKey)
{
  qInfo() << "Binding queue: " << QString::fromStdString(m_QueueName)
          << " to exchange: " << QString::fromStdString(m_ExchangeName)
          << " with binding key: " << QString::fromStdString(m_BindingKey)
          << " on channel: " << m_channel;

  const amqp_table_t emptyArgs = amqp_empty_table;
  amqp_queue_bind(m_AmqpConnection, m_channel,
                  amqp_cstring_bytes(m_QueueName.c_str()),
                  amqp_cstring_bytes(m_ExchangeName.c_str()),
                  amqp_cstring_bytes(m_BindingKey.c_str()),
                  emptyArgs);

  auto repl = amqp_get_rpc_reply(m_AmqpConnection);
  std::string msg = validation(repl, "Error binding: " + m_QueueName);
  if (!msg.empty())
    throw std::runtime_error(msg);
  else
    qInfo() << "Successfully bound queue: " << QString::fromStdString(m_QueueName);
}

RabbitmqBind::~RabbitmqBind()
{
  /*
  qInfo() << "Unbinding queue: " << QString::fromStdString(m_QueueName)
          << " from exchange: " << QString::fromStdString(m_ExchangeName)
          << " with binding key: " << QString::fromStdString(m_BindingKey)
          << " on channel: " << m_channel;

  auto shared = m_connection.lock();
  if (shared)
  {
    const amqp_table_t emptyArgs = amqp_empty_table;
    amqp_queue_unbind(m_AmqpConnection, m_channel,
                      amqp_cstring_bytes(m_QueueName.c_str()),
                      amqp_cstring_bytes(m_ExchangeName.c_str()),
                      amqp_cstring_bytes(m_BindingKey.c_str()),
                      emptyArgs);

    auto repl = amqp_get_rpc_reply(m_AmqpConnection);
    std::string msg = validation(repl, "Error unbinding: " + m_QueueName);
    if (msg.empty())
      qInfo() << "Successfully unbound queue: " << QString::fromStdString(m_QueueName);
  }
  else
  {
    qCritical() << "Error unbinding: undefined connection";
  }
  */
}

RabbitmqEnvelope::~RabbitmqEnvelope()
{
  amqp_destroy_envelope(&m_envelope);
}

std::string RabbitmqEnvelope::getMessage() const
{
  if (m_envelope.message.body.bytes == nullptr || m_envelope.message.body.len == 0)
    throw std::runtime_error("No message body found in the envelope");

  return std::string(static_cast<const char*>(m_envelope.message.body.bytes), m_envelope.message.body.len);
}
