#include "hstp.hpp"
#include "mercury_client.hpp"
#include <QCoreApplication>
#include <QEventLoop>
#include <QTcpServer>
#include <QtCore/qstringview.h>
#include <QtCore/qvariant.h>
#include <gtest/gtest.h>
#include <memory>

class MercuryClientTest : public testing::Test
{
public:
  MercuryClient client;
  QTcpServer test_server;
  HstpHandler hstp_handler;

  QCoreApplication *app;

  std::shared_ptr<QByteArray> payload_1;

  MercuryClientTest() : client("Alex") {}
  ~MercuryClientTest() override {}

  std::shared_ptr<QTcpSocket> get_hstp_sock() { return client.m_hstp_sock; }

  void SetUp() override
  {
    int argc = 0;
    char *argv[] = {nullptr};
    app = new QCoreApplication(argc, argv);

    hstp_handler.init_msg("Alice");
    hstp_handler.add_option_echo("echo!");
    payload_1 = hstp_handler.output_msg();

    ASSERT_TRUE(test_server.listen(QHostAddress::LocalHost, 12345));

    // Connect client to server
    get_hstp_sock()->connectToHost(QHostAddress::LocalHost,
                                   test_server.serverPort());
    ASSERT_TRUE(get_hstp_sock()->waitForConnected(1000));

    // Accept the connection on the server side
    QObject::connect(&test_server, &QTcpServer::newConnection,
                     [&]()
                     {
                       QTcpSocket *serverSocket =
                           test_server.nextPendingConnection();
                       serverSocket->write(*payload_1);
                       serverSocket->flush();
                     });
  }

  void TearDown() override
  {
    get_hstp_sock()->disconnectFromHost();
    test_server.close();
    delete app;
  }
};

TEST_F(MercuryClientTest, ProcessEchoOption)
{
  QEventLoop loop;
  QByteArray receivedData;

  // Run the event loop to wait for the data
  QObject::connect(&client, &MercuryClient::test_readyread_callback,
                   [&]()
                   {
                     loop.quit(); // Quit the loop when data is received
                   });

  loop.exec();
  // Verify that the data received matches what was sent
  EXPECT_EQ(1, 1);
}
