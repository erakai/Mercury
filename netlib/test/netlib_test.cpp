#include "mercury_client.hpp"
#include "mercury_server.hpp"
#include <QCoreApplication>
#include <gtest/gtest.h>
#include <random>

// For generating random data.
using random_bytes_engine =
    std::independent_bits_engine<std::default_random_engine, CHAR_BIT,
                                 unsigned char>;

class NetlibTest : public testing::Test
{
public:
  NetlibTest()
  {
    MINIMUM_LOG_LEVEL = 1;

    // How to use QCoreApplication non-blocking:
    // https://stackoverflow.com/questions/15202098/using-qapplication-in-a-non-blocking-way
    int argc = 0;
    char *argv[] = {nullptr};
    app = new QCoreApplication(argc, argv);

    server.set_ports(23333, 32222);
  }

  ~NetlibTest()
  {
    if (server.isListening())
      server.close_server();

    // if (client.isConnected())
    //   client.close();

    MINIMUM_LOG_LEVEL = -1;
    delete app;
  }

  QByteArray generate_random_data(int n_bytes)
  {
    std::vector<unsigned char> data(n_bytes);
    std::generate(begin(data), end(data), std::ref(rbe));

    return QByteArray(&data[0]);
  };

  random_bytes_engine rbe;

  QCoreApplication *app;

  MercuryServer server;
  // MercuryClient client;
};

TEST_F(NetlibTest, ServerClientBasic)
{
  ASSERT_TRUE(server.start_server());
  /*
  ASSERT_TRUE(client.connect(QHostAddress::LocalHost, server.get_tcp_port()));
  app->processEvents();
  EXPECT_EQ(server.getClients().size(), 1);

  // Test validation
  client.send_establishing.. (alias).
  app->processEvents();
  int client_id = (server.getClients().begin())->id;
  EXPECT_EQ(server.getClient(client_id).validated, true);
  EXPECT_EQ(server.getClient(client_id).alias, alias);

  // Test MFTP
  QByteArray audio_data = generate_random_data(20);
  QByteArray video_data = generate_random_data(20);
  QAudioBuffer audio(audio_data, QAudioFormat());
  QPixMap video;
  video.loadFromData(video_data);

  // Need to also test sending a bunch of frames out of order (in a different
  test case) to make sure client reorders them
  ASSERT_EQ(server.send_frame("test", audio, video), 1);
  app->processEvents();

  AV_Frame frame = client.pop_frame();
  recieved_audio = frame.audio;
  received_video = frame.video;
  EXPECT_EQ(audio_data, received_audio);
  EXPECT_EQ(video_data, received_video);

  // Test HSTP
  */
}

// Demonstrate some basic assertions, sanity check.
TEST(HelloWorldTest, SanityCheck)
{
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
