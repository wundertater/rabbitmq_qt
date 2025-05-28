#include "Client.h"
#include "Server.h"
#include "RabbitMQClient/RabbitmqConnection.h"
#include "Logger/Logger.h"

#include <gtest/gtest.h>

#include <thread>

namespace
{
  std::unique_ptr<Server> createServer(int heartbeat)
  {
    return std::make_unique<Server>(RabbitmqConnection::create(),
                                    "rabbitmq", 5672,
                                    "guest", "guest", heartbeat, "/",
                                    "test_exchange", "response_queue", "request_queue");
  }


  std::unique_ptr<Client> createClient(int heartbeat)
  {
    return std::make_unique<Client>(RabbitmqConnection::create(),
                                    "rabbitmq", 5672,
                                    "guest", "guest", heartbeat, "/",
                                    "test_exchange", "response_queue", "request_queue");
  }

  void runClient(int requestValue, const std::atomic<bool>& running)
  {
    int expected = Server::generateResponseValue(requestValue);
    auto client = createClient(0);
    client->sendRequest(requestValue);

    bool success = false;
    while (running)
    {
      auto response = client->getResponse(std::chrono::milliseconds(100));
      success = response.first;
      if (success)
      {
        int responseValue = response.second;
        EXPECT_EQ(responseValue, expected);
        break;
      }
    }
    EXPECT_TRUE(success);
  }

  void runServer(const std::atomic<bool>& running)
  {
    auto server = createServer(0);
    while(running)
      server->processRequestResponseCycle(std::chrono::milliseconds(100));
  }
}

class IntegrationTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite()
  {
    Logger::setupLogging("logs.txt", QtInfoMsg);
  }
};


TEST_F(IntegrationTest, ServerBeforeClient)
{
  auto server = createServer(0);
  std::thread serverThread(&Server::processRequestResponseCycle, server.get(), std::chrono::milliseconds(200));

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::thread clientThread([]()
  {
    int requestValue = 0;
    int expected = Server::generateResponseValue(requestValue);
    auto client = createClient(0);
    client->sendRequest(requestValue);

    auto response = client->getResponse(std::chrono::milliseconds(100));

    EXPECT_TRUE(response.first);
    EXPECT_EQ(response.second, expected);
  });

  clientThread.join();
  serverThread.join();
}

TEST_F(IntegrationTest, ClientBeforeServer)
{
  std::thread clientThread([]()
  {
    int requestValue = 42;
    int expected = Server::generateResponseValue(requestValue);
    auto client = createClient(0);
    client->sendRequest(requestValue);

    auto response = client->getResponse(std::chrono::milliseconds(100));

    EXPECT_TRUE(response.first);
    EXPECT_EQ(response.second, expected);
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  auto server = createServer(0);
  std::thread serverThread(&Server::processRequestResponseCycle, server.get(), std::chrono::milliseconds(200));

  clientThread.join();
  serverThread.join();
}

TEST_F(IntegrationTest, ServerBeforeMultipleClients)
{
  const int clientCount = 5;
  std::vector<std::thread> clients;
  std::atomic<bool> running(true);

  std::thread serverThread(runServer, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  for (int i = 0; i < clientCount; ++i)
    clients.emplace_back(runClient, i * 10, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  running = false;

  for (auto& client : clients)
      client.join();

  serverThread.join();
}


TEST_F(IntegrationTest, MultipleClientsBeforeServer)
{
  const int clientCount = 5;
  std::vector<std::thread> clients;
  std::atomic<bool> running(true);

  for (int i = 0; i < clientCount; ++i)
    clients.emplace_back(runClient, -i * 10, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::thread serverThread(runServer, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  running = false;

  serverThread.join();

  for (auto& client : clients)
      client.join();
}


TEST_F(IntegrationTest, ServerBetweenClients)
{
  std::vector<std::thread> clients;
  std::atomic<bool> running(true);
  clients.emplace_back(runClient, 0, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::thread serverThread(runServer, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  clients.emplace_back(runClient, -1000, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  running = false;

  serverThread.join();
  for (auto& client : clients)
      client.join();
}

TEST_F(IntegrationTest, ServerBetweenMultipleClients)
{
  const int clientCount = 5;
  std::vector<std::thread> clients;
  std::atomic<bool> running(true);

  for (int i = 0; i < clientCount; ++i)
    clients.emplace_back(runClient, i, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::thread serverThread(runServer, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  for (int i = 0; i < clientCount; ++i)
    clients.emplace_back(runClient, i + 1000, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  running = false;

  serverThread.join();
  for (auto& client : clients)
      client.join();
}

TEST_F(IntegrationTest, ClientsSleep)
{
  const int clientCount = 5;
  std::vector<std::thread> clients;
  std::atomic<bool> running(true);

  for (int i = 0; i < clientCount; ++i)
  {
    clients.emplace_back([i, &running]()
    {
      int requestValue = 100 + i;
      int expected = Server::generateResponseValue(requestValue);
      auto client = createClient(0);
      client->sendRequest(requestValue);

      std::this_thread::sleep_for(std::chrono::milliseconds((i + 1) * 100 + 500));

      bool success = false;
      while (running)
      {
        auto response = client->getResponse(std::chrono::milliseconds(100));
        success = response.first;
        if (success)
        {
          int responseValue = response.second;
          EXPECT_EQ(responseValue, expected);
          break;
        }
      }
      EXPECT_TRUE(success);
    });
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  std::thread serverThread(runServer, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  for (int i = 0; i < clientCount; ++i)
    clients.emplace_back(runClient, i * 12345678, std::ref(running));

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  running = false;

  serverThread.join();
  for (auto& client : clients)
      client.join();
}

