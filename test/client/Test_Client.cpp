#include "mocks.h"

#include "Client.h"
#include "protocol/Messages.pb.h"
#include "Logger/Logger.h"

#include <gtest/gtest.h>

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Throw;
using testing::ByMove;

class ClientTest : public ::testing::Test
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
    EXPECT_CALL(*mockConnection, basicConsume(_, _, false, false))
         .Times(1);
   }

};

TEST_F(ClientTest, SendRequest_Success)
{
  auto client = std::make_unique<Client>(mockConnection,
                                         "localhost", 5672,
                                         "guest", "guest",
                                         0, "/",
                                         "testExchange", "responseQueue", "requestQueue");

  // Ожидаемый запрос
  int req = 42;
  TestTask::Messages::Request expectedRequest;
  expectedRequest.set_id(client->getId().toString().toStdString());
  expectedRequest.set_req(req);

  std::string expectedStr;
  expectedRequest.SerializeToString(&expectedStr);

  // проверка вызова publishMessage
  EXPECT_CALL(*mockConnection, publishMessage(_, _, _, expectedStr))
          .Times(1);

  client->sendRequest(req);
}

TEST_F(ClientTest, SendRequest_Zero)
{
  auto client = std::make_unique<Client>(mockConnection,
                                         "localhost", 5672,
                                         "guest", "guest",
                                         0, "/",
                                         "testExchange", "responseQueue", "requestQueue");

  // Ожидаемый запрос
  int req = 0;
  TestTask::Messages::Request expectedRequest;
  expectedRequest.set_id(client->getId().toString().toStdString());
  expectedRequest.set_req(req);

  std::string expectedStr;
  expectedRequest.SerializeToString(&expectedStr);

  // проверка вызова publishMessage
  EXPECT_CALL(*mockConnection, publishMessage(_, _, _, expectedStr))
          .Times(1);

  client->sendRequest(req);
}

TEST_F(ClientTest, SendRequest_Error)
{
  auto client = std::make_unique<Client>(mockConnection,
                                         "localhost", 5672,
                                         "guest", "guest",
                                         0, "/",
                                         "testExchange", "responseQueue", "requestQueue");

  // publishMessage кинет исключение
  EXPECT_CALL(*mockConnection, publishMessage(_, _, _, _))
          .WillOnce(Throw(std::runtime_error("Publish error")));

  EXPECT_THROW(client->sendRequest(15), std::runtime_error);
}

TEST_F(ClientTest, GetResponse_Success)
{
  auto client = std::make_unique<Client>(mockConnection,
                                         "localhost", 5672,
                                         "guest", "guest",
                                         0, "/",
                                         "testExchange", "responseQueue", "requestQueue");
  int expectedRes = 18;
  std::chrono::milliseconds timeout(1000);

  // создаем ответ от сервера
  TestTask::Messages::Response response;
  response.set_id(client->getId().toString().toStdString());
  response.set_res(expectedRes);
  std::string serializedResponse;
  response.SerializeToString(&serializedResponse);

  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(serializedResponse));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  // также ожидаем подтверждение
  EXPECT_CALL(*mockConnection, ack(_))
      .Times(1);

  auto result = client->getResponse(timeout);
  EXPECT_TRUE(result.first);
  EXPECT_EQ(result.second, expectedRes);
}

TEST_F(ClientTest, GetResponse_Zero)
{
  auto client = std::make_unique<Client>(mockConnection,
                                         "localhost", 5672,
                                         "guest", "guest",
                                         0, "/",
                                         "testExchange", "responseQueue", "requestQueue");
  int expectedRes = 0;
  std::chrono::milliseconds timeout(1000);

  // создаем ответ от сервера
  TestTask::Messages::Response response;
  response.set_id(client->getId().toString().toStdString());
  response.set_res(expectedRes);
  std::string serializedResponse;
  response.SerializeToString(&serializedResponse);

  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(serializedResponse));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  // также ожидаем подтверждение
  EXPECT_CALL(*mockConnection, ack(_))
      .Times(1);

  auto result = client->getResponse(timeout);
  EXPECT_TRUE(result.first);
  EXPECT_EQ(result.second, expectedRes);
}

TEST_F(ClientTest, GetResponse_InvalidResponse)
{
  // создаем клиента
  auto client = std::make_unique<Client>(mockConnection,
                                          "localhost", 5672,
                                          "guest", "guest",
                                          0, "/",
                                          "testExchange", "responseQueue", "requestQueue");

  std::chrono::milliseconds timeout(1000);
  // создаем неправильный ответ
  std::string invalidResponse = "invalid_response";

  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(invalidResponse));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  EXPECT_THROW(client->getResponse(timeout), std::runtime_error);
}

TEST_F(ClientTest, GetResponse_ConsumeError)
{
  auto client = std::make_unique<Client>(mockConnection,
                                          "localhost", 5672,
                                          "guest", "guest",
                                          0, "/",
                                          "testExchange", "responseQueue", "requestQueue");

  std::chrono::milliseconds timeout(1000);

  // timedConsumeMessage кинет исключение
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
          .WillOnce(Throw(std::runtime_error("")));

  EXPECT_THROW(client->getResponse(timeout), std::runtime_error);
}

TEST_F(ClientTest, GetResponse_WrongId)
{
  auto client = std::make_unique<Client>(mockConnection,
                                         "localhost", 5672,
                                         "guest", "guest",
                                         0, "/",
                                         "testExchange", "responseQueue", "requestQueue");

  std::chrono::milliseconds timeout(1000);

  // создаем ответ от сервера с чужим id
  TestTask::Messages::Response response;
  response.set_id(client->getId().toString().toStdString() + "_other");
  response.set_res(8080);
  std::string serializedResponse;
  response.SerializeToString(&serializedResponse);

  // процесс передачи сообщения: передаем mockEnvelope, который вернет сообщение
  auto mockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
  EXPECT_CALL(*mockEnvelope, getMessage())
      .WillOnce(Return(serializedResponse));
  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(std::move(mockEnvelope))));

  // также ожидаем отклонения
  EXPECT_CALL(*mockConnection, reject(_))
      .Times(1);

  auto result = client->getResponse(timeout);
  EXPECT_FALSE(result.first);
  EXPECT_EQ(result.second, 0);
}

TEST_F(ClientTest, GetResponse_Timeout)
{
  auto client = std::make_unique<Client>(mockConnection,
                                         "localhost", 5672,
                                         "guest", "guest",
                                         0, "/",
                                         "testExchange", "responseQueue", "requestQueue");

  std::chrono::milliseconds timeout(1000);

  EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
      .WillOnce(Return(ByMove(nullptr)));

  auto result = client->getResponse(timeout);
  EXPECT_FALSE(result.first);
  EXPECT_EQ(result.second, 0);
}

TEST_F(ClientTest, GetResponse_MultipleResponses)
{
    auto client = std::make_unique<Client>(mockConnection,
                                            "localhost", 5672,
                                            "guest", "guest",
                                            0, "/",
                                            "testExchange", "responseQueue", "requestQueue");

    std::chrono::milliseconds timeout(1000);

    // Создаем первый ответ с неправильным id
    TestTask::Messages::Response wrongResponse;
    wrongResponse.set_id(client->getId().toString().toStdString() + "_wrong");
    wrongResponse.set_res(40);
    std::string serializedWrongResponse;
    wrongResponse.SerializeToString(&serializedWrongResponse);

    // Создаем второй ответ с правильным id
    int expectedRes = 42;
    TestTask::Messages::Response correctResponse;
    correctResponse.set_id(client->getId().toString().toStdString());
    correctResponse.set_res(expectedRes);
    std::string serializedCorrectResponse;
    correctResponse.SerializeToString(&serializedCorrectResponse);

    auto wrongMockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
    EXPECT_CALL(*wrongMockEnvelope, getMessage())
        .WillOnce(Return(serializedWrongResponse));

    auto correctMockEnvelope = std::make_unique<MockRabbitmqEnvelope>();
    EXPECT_CALL(*correctMockEnvelope, getMessage())
        .WillOnce(Return(serializedCorrectResponse));

    EXPECT_CALL(*mockConnection, timedConsumeMessage(timeout))
        .WillOnce(Return(ByMove(std::move(wrongMockEnvelope))))
        .WillOnce(Return(ByMove(std::move(correctMockEnvelope))));

    EXPECT_CALL(*mockConnection, reject(_))
        .Times(1);

    EXPECT_CALL(*mockConnection, ack(_))
        .Times(1);

    // Обрабатываем оба ответа
    auto resultWrong = client->getResponse(timeout);
    EXPECT_FALSE(resultWrong.first);
    EXPECT_EQ(resultWrong.second, 0);

    auto resultCorrect = client->getResponse(timeout);
    EXPECT_TRUE(resultCorrect.first);
    EXPECT_EQ(resultCorrect.second, expectedRes);
}
