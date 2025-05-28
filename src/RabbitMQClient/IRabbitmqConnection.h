#ifndef IRABBITMQCONNECTION_H
#define IRABBITMQCONNECTION_H

#include <memory>
#include <string>
#include <chrono>

class RabbitmqSocket;
class RabbitmqChannel;
class RabbitmqExchange;
class RabbitmqQueue;
class RabbitmqBind;
class IRabbitmqEnvelope;

class IRabbitmqConnection : public std::enable_shared_from_this<IRabbitmqConnection>
{
public:
  virtual ~IRabbitmqConnection() = default;

  virtual std::unique_ptr<RabbitmqSocket> openSocket(const std::string &host, int port) = 0;

  virtual void login(const std::string &login, const std::string &password,
                       int heartbeatInSeconds, const std::string& vhost) = 0;

  virtual std::unique_ptr<RabbitmqChannel> openChannel() = 0;

  virtual std::unique_ptr<RabbitmqExchange> declareExchange(const RabbitmqChannel& channel,
                                                            const std::string& exchangeName,
                                                            const std::string& exchangeType) = 0;

  virtual std::unique_ptr<RabbitmqQueue> declareQueue(const RabbitmqChannel& channel, const std::string& queueName) = 0;

  virtual std::unique_ptr<RabbitmqBind> bind(const RabbitmqChannel &channel, const RabbitmqQueue &queue,
                                             const RabbitmqExchange &exchange, const std::string &bindingKey) = 0;

  virtual void basicConsume(const RabbitmqChannel &channel, const RabbitmqQueue &queue, bool noAsk, bool exclusive) = 0;

  virtual void publishMessage(const RabbitmqChannel& channel, const RabbitmqExchange& exchange,
                              const RabbitmqBind& binding, std::string message) = 0;

  virtual void ack(const IRabbitmqEnvelope &envelope) = 0;
  virtual void reject(const IRabbitmqEnvelope &envelope) = 0;

  virtual std::unique_ptr<IRabbitmqEnvelope> consumeMessage() = 0;
  virtual std::unique_ptr<IRabbitmqEnvelope> timedConsumeMessage(std::chrono::milliseconds timeoutMillis) = 0;
protected:
  virtual std::unique_ptr<IRabbitmqEnvelope> consumeMessageInternal(struct timeval* timeout) = 0;
};

#endif
