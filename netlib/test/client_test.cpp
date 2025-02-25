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
    hstp_handler.add_option_establishment(true, 12345);
    hstp_handler.add_option_chat("Chatter", "Poggers!");
    payload_1 = hstp_handler.output_msg();

    ASSERT_TRUE(test_server.listen(QHostAddress::LocalHost, 10202));

    get_hstp_sock()->connectToHost(QHostAddress::LocalHost,
                                   test_server.serverPort());
    ASSERT_TRUE(get_hstp_sock()->waitForConnected(1000));

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

TEST_F(MercuryClientTest, ProcessChatEstablishmentOption)
{
  QEventLoop loop;
  QByteArray receivedData;

  QObject::connect(
      client.hstp_processor().get(), &HstpProcessor::received_establishment,
      [&](const char alias[ALIAS_SIZE], bool is_start, uint16_t mftp_port)
      {
        EXPECT_STREQ(alias, "Alice");
        EXPECT_EQ(is_start, true);
        EXPECT_EQ(mftp_port, 12345);
      });

  QObject::connect(client.hstp_processor().get(), &HstpProcessor::received_chat,
                   [&](const char alias[ALIAS_SIZE],
                       const char alias_of_chatter[ALIAS_SIZE],
                       const std::string &chat)
                   {
                     EXPECT_STREQ(alias, "Alice");
                     EXPECT_STREQ(alias_of_chatter, "Chatter");
                     EXPECT_STREQ(chat.c_str(), "Poggers!");
                     loop.quit(); // Quit the loop when data is received
                   });

  loop.exec();
}
