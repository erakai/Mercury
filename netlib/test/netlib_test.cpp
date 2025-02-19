#include "mercury_client.hpp"
#include "mercury_server.hpp"
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

    server.set_ports(23333, 32222);
  }

  ~NetlibTest()
  {
    if (server.isListening())
      server.close_server();

    // if (client.isConnected())
    //   client.close();

    MINIMUM_LOG_LEVEL = -1;
  }

  QByteArray generate_random_data(int n_bytes)
  {
    std::vector<unsigned char> data(n_bytes);
    std::generate(begin(data), end(data), std::ref(rbe));

    return QByteArray(&data[0]);
  };

  random_bytes_engine rbe;
  MercuryServer server;
  // MercuryClient client;
};

TEST_F(NetlibTest, ServerClientBasic)
{
  ASSERT_TRUE(server.start_server());

  /*
  ASSERT_TRUE(client.connect(QHostAddress::LocalHost, server.get_tcp_port()));
  EXPECT_EQ(server.getClients().size(), 1);

  // Test validation
  client.send_establishing.. (alias).
  int client_id = (server.getClients().begin())->id;
  EXPECT_EQ(server.getClient(client_id).validated, true);
  EXPECT_EQ(server.getClient(client_id).alias, alias);

  // Test MFTP
  QAudioBuffer audio(generate_random_data(20), QAudioFormat());
  QPixMap video;
  video.loadFromData(generate_random_data(20));

  // Need to also test sending a bunch of frames out of order (in a different
  test case)
  ASSERT_EQ(server.send_frame("test", audio, video), 1);

  AV_Frame frame = client.pop_frame();
  recieved_audio..
  received_video..
  EXPECT_EQ(audio, received_audio);
  EXPECT_EQ(video, received_video);

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
