#ifndef MOCKS_H
#define MOCKS_H

#include "RabbitMQClient/IRabbitmqConnection.h"
#include "RabbitMQClient/rabbitmqEntities.h"

#include <gmock/gmock.h>

class MockRabbitmqEnvelope : public IRabbitmqEnvelope {
public:
  MOCK_METHOD(amqp_envelope_t*, get, (), (override));
  MOCK_METHOD(amqp_channel_t, getChannel, (), (const, override));
  MOCK_METHOD(uint64_t, getDeliveryTag, (), (const, override));
  MOCK_METHOD(std::string, getMessage, (), (const, override));
};


class MockRabbitmqConnection : public IRabbitmqConnection {
public:
  MOCK_METHOD(std::unique_ptr<RabbitmqSocket>,
              openSocket,
              (const std::string &host, int port),
              (override));

  MOCK_METHOD(void,
              login,
              (const std::string &login, const std::string &password, int heartbeatInSeconds, const std::string& vhost),
              (override));

  MOCK_METHOD(std::unique_ptr<RabbitmqChannel>,
              openChannel,
              (),
              (override));

  MOCK_METHOD(std::unique_ptr<RabbitmqExchange>,
              declareExchange,
              (const RabbitmqChannel& channel, const std::string& exchangeName, const std::string& exchangeType),
              (override));

  MOCK_METHOD(std::unique_ptr<RabbitmqQueue>,
              declareQueue,
              (const RabbitmqChannel &channel, const std::string& queueName),
              (override));

  MOCK_METHOD(std::unique_ptr<RabbitmqBind>,
              bind,
              (const RabbitmqChannel &channel, const RabbitmqQueue &queue, const RabbitmqExchange &exchange, const std::string &bindingKey),
              (override));

  MOCK_METHOD(void,
              basicConsume,
              (const RabbitmqChannel &channel, const RabbitmqQueue &queue, bool noAsk, bool exclusive),
              (override));

  MOCK_METHOD(void,
              publishMessage,
              (const RabbitmqChannel& channel, const RabbitmqExchange& exchange, const RabbitmqBind& binding, std::string message),
              (override));

  MOCK_METHOD(void,
              ack,
              (const IRabbitmqEnvelope &envelope),
              (override));

  MOCK_METHOD(void,
              reject,
              (const IRabbitmqEnvelope &envelope),
              (override));

  MOCK_METHOD(std::unique_ptr<IRabbitmqEnvelope>,
              consumeMessage,
              (),
              (override));
  MOCK_METHOD(std::unique_ptr<IRabbitmqEnvelope>,
              timedConsumeMessage,
              (std::chrono::milliseconds timeoutMillis),
              (override));
  MOCK_METHOD(std::unique_ptr<IRabbitmqEnvelope>,
              consumeMessageInternal,
              (struct timeval* timeout),
              (override));
};

#endif
