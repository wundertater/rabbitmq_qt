#ifndef RABBITMQENTITIES_H
#define RABBITMQENTITIES_H

#include "IRabbitmqConnection.h"

#include <amqp.h>
#include <amqp_tcp_socket.h>


class RabbitmqSocket
{
public:
  RabbitmqSocket(amqp_connection_state_t connection, const std::string &host, int port);
  ~RabbitmqSocket() = default;

  RabbitmqSocket(const RabbitmqSocket&) = delete;
  RabbitmqSocket& operator=(const RabbitmqSocket&) = delete;
private:
  amqp_socket_t* m_socket = nullptr;
};

class RabbitmqChannel
{
public:
  RabbitmqChannel(std::shared_ptr<IRabbitmqConnection> connection,
                  amqp_connection_state_t amqpConnection, amqp_channel_t channel);
  ~RabbitmqChannel();

  amqp_channel_t getId() const {return m_channel;}

  RabbitmqChannel(const RabbitmqChannel&) = delete;
  RabbitmqChannel& operator=(const RabbitmqChannel&) = delete;
private:
  std::weak_ptr<IRabbitmqConnection> m_connection;
  amqp_connection_state_t m_AmqpConnection = nullptr;
  amqp_channel_t m_channel;
};

class RabbitmqExchange
{
public:
  RabbitmqExchange(std::shared_ptr<IRabbitmqConnection> connection, amqp_connection_state_t amqpConnection,
                   amqp_channel_t channel, const std::string& exchangeName, const std::string& exchangeType);
  ~RabbitmqExchange();

  RabbitmqExchange(const RabbitmqChannel&) = delete;
  RabbitmqExchange& operator=(const RabbitmqChannel&) = delete;

  std::string getName() const {return m_ExchangeName;}
private:
  std::weak_ptr<IRabbitmqConnection> m_connection;
  amqp_connection_state_t m_AmqpConnection = nullptr;
  amqp_channel_t m_channel;
  std::string m_ExchangeName;
};

class RabbitmqQueue
{
public:
  RabbitmqQueue(std::shared_ptr<IRabbitmqConnection> connection, amqp_connection_state_t amqpConnection,
                amqp_channel_t channel, const std::string& queueName);
  ~RabbitmqQueue();

  RabbitmqQueue(const RabbitmqChannel&) = delete;
  RabbitmqQueue& operator=(const RabbitmqChannel&) = delete;

  std::string getName() const {return m_QueueName;}
private:
  std::weak_ptr<IRabbitmqConnection> m_connection;
  amqp_connection_state_t m_AmqpConnection = nullptr;
  amqp_channel_t m_channel;
  std::string m_QueueName;
};

class RabbitmqBind
{
public:
  RabbitmqBind(std::shared_ptr<IRabbitmqConnection> connection, amqp_connection_state_t amqpConnection,
               amqp_channel_t channel, const std::string& queueName,
               const std::string& exchangeName, const std::string& bindingKey);
  ~RabbitmqBind();

  RabbitmqBind(const RabbitmqChannel&) = delete;
  RabbitmqBind& operator=(const RabbitmqChannel&) = delete;

  std::string getBindingKey() const {return m_BindingKey;}
private:
  std::weak_ptr<IRabbitmqConnection> m_connection;
  amqp_connection_state_t m_AmqpConnection = nullptr;
  amqp_channel_t m_channel;
  std::string m_QueueName;
  std::string m_ExchangeName;
  std::string m_BindingKey;
};


class IRabbitmqEnvelope
{
public:
  virtual ~IRabbitmqEnvelope() = default;

  virtual amqp_envelope_t* get() = 0;
  virtual amqp_channel_t getChannel() const = 0;
  virtual uint64_t getDeliveryTag() const = 0;
  virtual std::string getMessage() const = 0;
};

class RabbitmqEnvelope : public IRabbitmqEnvelope
{
public:
  RabbitmqEnvelope() = default;
  ~RabbitmqEnvelope();

  RabbitmqEnvelope(const RabbitmqEnvelope&) = delete;
  RabbitmqEnvelope& operator=(const RabbitmqEnvelope&) = delete;

  amqp_envelope_t* get() override {return &m_envelope;}
  amqp_channel_t getChannel() const override {return m_envelope.channel;}
  uint64_t getDeliveryTag() const override {return m_envelope.delivery_tag;}
  std::string getMessage() const override;
private:
  amqp_envelope_t m_envelope;
};

#endif
