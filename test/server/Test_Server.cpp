#include "mocks.h"

#include "Server.h"
#include "protocol/Messages.pb.h"
#include "Logger/Logger.h"

#include <gtest/gtest.h>

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Throw;
using testing::ByMove;

class ServerTest : public ::testing::Test
{
protected:
  std::shared_ptr<MockRabbitmqConnection> mockConnection;

  static void SetUpTestSuite()
  {
    Logger::setupLogging("logs.txt", QtInfoMsg);
  }

  void SetUp() override
  {
    mockConnection = std::make_shared<MockRabbitmqConnection>();
    SetUpMockConnectionExpectations(mockConnection);
  }

  void SetUpMockConnectionExpectations(const std::shared_ptr<MockRabbitmqConnection>& connection)
  {
    EXPECT_CALL(*mockConnection, openSocket("localhost", 5672))
        .WillOnce(Return(ByMove(nullptr)));
    EXPECT_CALL(*mockConnection, login("guest", "guest", 0, "/"))
        .Times(1);
    EXPECT_CALL(*mockConnection, openChannel())
         .WillOnce(Return(ByMove(nullptr)));
    EXPECT_CALL(*mockConnection, declareExchange(_, "testExchange", "direct"))
         .WillOnce(Return(ByMove(nullptr)));
    EXPECT_CALL(*mockConnection, declareQueue(_, "responseQueue"))
         .WillOnce(Return(ByMove(nullptr)));
    EXPECT_CALL(*mockConnection, declareQueue(_, "requestQueue"))
         .WillOnce(Return(ByMove(nullptr)));
    EXPECT_CALL(*mockConnection, bind(_, _, _, "responseQueue"))
         .WillOnce(Return(ByMove(nullptr)));
    EXPECT_CALL(*mockConnection, bind(_, _, _, "requestQueue"))
         .WillOnce(Return(ByMove(nullptr)));
    EXPECT_CALL(*mockConnection, basicConsume(_, _, true, true))
         .Times(1);
   }

};

TEST_F(ServerTest, ProcessRequestResponseCycle_Success)
{
  std::chrono::milliseconds timeout(200);
  // запрос от клиента
  TestTask::Messages::Request request;
  request.set_id("req-1");
  request.set_req(5);
  std::string serializedRequest;
  request.SerializeToString(&serializedRequest);

  // ожидаемый ответ
  TestTask::Messages::Response expectedResponse;
  expectedResponse.set_id("req-1");
  expectedResponse.set_res(10);
  std::string expected;
  expectedResponse.SerializeToString(&expected);


  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(serializedRequest));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  // должен быть вызван publishMessage с сообщением expected
  EXPECT_CALL(*mockConnection, publishMessage(_, _, _, expected))
      .Times(1);

  auto server = std::make_unique<Server>(mockConnection,
                                    "localhost", 5672,
                                    "guest", "guest",
                                    0, "/",
                                    "testExchange", "responseQueue", "requestQueue");
  server->processRequestResponseCycle(timeout);
}

TEST_F(ServerTest, ProcessRequestResponseCycle_Zero)
{
  std::chrono::milliseconds timeout(200);
  // запрос от клиента
  TestTask::Messages::Request request;
  request.set_id("req-1");
  request.set_req(0);
  std::string serializedRequest;
  request.SerializeToString(&serializedRequest);

  // ожидаемый ответ
  TestTask::Messages::Response expectedResponse;
  expectedResponse.set_id("req-1");
  expectedResponse.set_res(0);
  std::string expected;
  expectedResponse.SerializeToString(&expected);

  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(serializedRequest));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  // должен быть вызван publishMessage с сообщением expected
  EXPECT_CALL(*mockConnection, publishMessage(_, _, _, expected))
      .Times(1);

  auto server = std::make_unique<Server>(mockConnection,
                                    "localhost", 5672,
                                    "guest", "guest",
                                    0, "/",
                                    "testExchange", "responseQueue", "requestQueue");
  server->processRequestResponseCycle(timeout);
}

TEST_F(ServerTest, ProcessRequestResponseCycle_WrongMsg)
{
  std::chrono::milliseconds timeout(200);
  // запрос от клиента
  TestTask::Messages::Request request;
  request.set_id("req-1");
  request.set_req(5);
  std::string serializedRequest;
  request.SerializeToString(&serializedRequest);
  serializedRequest[0] = 0; // портим запрос

  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(serializedRequest));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  auto server = std::make_unique<Server>(mockConnection,
                                    "localhost", 5672,
                                    "guest", "guest",
                                    0, "/",
                                    "testExchange", "responseQueue", "requestQueue");
  EXPECT_THROW(server->processRequestResponseCycle(timeout), std::runtime_error);
}

TEST_F(ServerTest, ProcessRequestResponseCycle_EmptyMsg)
{
  std::chrono::milliseconds timeout(200);
  // запрос от клиента
  std::string serializedRequest;

  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(serializedRequest));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  auto server = std::make_unique<Server>(mockConnection,
                                    "localhost", 5672,
                                    "guest", "guest",
                                    0, "/",
                                    "testExchange", "responseQueue", "requestQueue");
  EXPECT_THROW(server->processRequestResponseCycle(timeout), std::runtime_error);
}

TEST_F(ServerTest, ProcessRequestResponseCycle_PublishError)
{
  std::chrono::milliseconds timeout(200);
  // запрос от клиента
  TestTask::Messages::Request request;
  request.set_id("req-1");
  request.set_req(5);
  std::string serializedRequest;
  request.SerializeToString(&serializedRequest);

  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(serializedRequest));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  // метод publishMessage кинет исключение
  EXPECT_CALL(*mockConnection, publishMessage(_, _, _, _))
      .WillOnce(Throw(std::runtime_error("")));

  auto server = std::make_unique<Server>(mockConnection,
                                    "localhost", 5672,
                                    "guest", "guest",
                                    0, "/",
                                    "testExchange", "responseQueue", "requestQueue");
  EXPECT_THROW(server->processRequestResponseCycle(timeout), std::runtime_error);
}

TEST_F(ServerTest, ProcessRequestResponseCycle_Timeout)
{
  std::chrono::milliseconds timeout(200);

  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(nullptr)));

  auto server = std::make_unique<Server>(mockConnection,
                                    "localhost", 5672,
                                    "guest", "guest",
                                    0, "/",
                                    "testExchange", "responseQueue", "requestQueue");
  server->processRequestResponseCycle(timeout);
}
