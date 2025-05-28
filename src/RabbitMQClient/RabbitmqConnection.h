#ifndef RABBITMQCONNECTION_H
#define RABBITMQCONNECTION_H

#include "IRabbitmqConnection.h"

#include <amqp.h>

class RabbitmqConnection : public IRabbitmqConnection
{
  class Private;
public:
  RabbitmqConnection(Private);
  virtual ~RabbitmqConnection();

  RabbitmqConnection (const RabbitmqConnection &) = delete;
  RabbitmqConnection & operator=(const RabbitmqConnection &) = delete;

  RabbitmqConnection (RabbitmqConnection &&) = default;
  RabbitmqConnection & operator=(RabbitmqConnection &&) = default;

  static std::shared_ptr<RabbitmqConnection> create();
  std::shared_ptr<IRabbitmqConnection> share();

  std::unique_ptr<RabbitmqSocket> openSocket(const std::string &host, int port) override;

  void login(const std::string &login, const std::string &password,
             int heartbeatInSeconds, const std::string& vhost) override;

  std::unique_ptr<RabbitmqChannel> openChannel() override;

  std::unique_ptr<RabbitmqExchange> declareExchange(const RabbitmqChannel& channel,
                                                    const std::string& exchangeName,
                                                    const std::string& exchangeType) override;

  std::unique_ptr<RabbitmqQueue> declareQueue(const RabbitmqChannel& channel,
                                              const std::string& queueName) override;

  std::unique_ptr<RabbitmqBind> bind(const RabbitmqChannel &channel, const RabbitmqQueue &queue,
                                     const RabbitmqExchange &exchange, const std::string &bindingKey) override;

  void basicConsume(const RabbitmqChannel &channel, const RabbitmqQueue &queue, bool noAsk, bool exclusive) override;

  void publishMessage(const RabbitmqChannel& channel, const RabbitmqExchange& exchange,
                      const RabbitmqBind& binding, std::string message) override;

  void ack(const IRabbitmqEnvelope &envelope) override;
  void reject(const IRabbitmqEnvelope &envelope) override;

  std::unique_ptr<IRabbitmqEnvelope> consumeMessage() override;
  std::unique_ptr<IRabbitmqEnvelope> timedConsumeMessage(std::chrono::milliseconds timeoutMillis) override;

protected:
  std::unique_ptr<IRabbitmqEnvelope> consumeMessageInternal(struct timeval* timeout) override;

private:
  amqp_connection_state_t m_connection = nullptr;
  amqp_channel_t m_freeChannelId = 1;

  struct Private{ explicit Private() = default; };
};

#endif
