#include "mercury_client.hpp"
#include "mercury_server.hpp"
#include <QApplication>
#include <QBuffer>
#include <QScreen>
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
      : client(MercuryClient("ClientAlias")),
        server(MercuryServer("ServerAlias"))
  {
    MINIMUM_LOG_LEVEL = 1;

    // How to use QCoreApplication non-blocking:
    // https://stackoverflow.com/questions/15202098/using-qapplication-in-a-non-blocking-way
    int argc = 0;
    char *argv[] = {nullptr};
    app = new QGuiApplication(argc, argv);

    server.set_ports(server_tcp, server_udp);
  }

  ~NetlibTest()
  {
    if (server.isListening())
      server.close_server();

    client.disconnect();

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

  QGuiApplication *app;

  MercuryClient client;
  MercuryServer server;
  int server_tcp = 23333;
  int server_udp = 32222;
  int client_tcp = 43333;
  int client_udp = 34444;
};

/*
This tests this sequence of events:

1. Server starts listening
2. Client establishes a connection with the server and sends an establishment
HSTP message
3. Server validates the client in its client list
4. Server sends a MFTP message to every client
5. Client receives the MFTP message and we confirm it is uncorrupted
*/
TEST_F(NetlibTest, ServerClientBasic)
{
  ASSERT_TRUE(server.start_server());
  ASSERT_TRUE(client.establish_connection(QHostAddress::LocalHost, server_tcp,
                                          client_udp));
  app->processEvents(QEventLoop::AllEvents, QDeadlineTimer(100));
  ASSERT_EQ(server.get_clients().size(), 1);

  // Test validation
  int client_id = (server.get_clients().begin())->second.id;
  EXPECT_EQ(server.get_client(client_id).validated, true);
  EXPECT_EQ(server.get_client(client_id).alias, client.get_alias());

  // Test MFTP
  QByteArray audio_data = generate_random_data(20);
  QAudioBuffer audio(audio_data, QAudioFormat());

  // Get screen for test video data
  QPixmap video = QGuiApplication::primaryScreen()->grabWindow(0);
  QByteArray video_data;
  QBuffer buffer(&video_data);
  buffer.open(QIODevice::WriteOnly);
  ASSERT_TRUE(video.save(&buffer, "PNG", 0));
  printf("Size of data: %lld\n", video_data.size());

  // Need to also test sending a bunch of frames out of order (in a different
  // test case) to make sure client reorders them
  ASSERT_EQ(server.send_frame("test", audio, video), 1);
  app->processEvents(QEventLoop::AllEvents, QDeadlineTimer(100));

  AV_Frame frame = client.retrieve_next_frame();
  QByteArray received_audio = frame.audio;
  QByteArray received_video = frame.video;
  EXPECT_EQ(audio_data, received_audio);
  EXPECT_EQ(video_data, received_video);
}

// Demonstrate some basic assertions, sanity check.
TEST(HelloWorldTest, SanityCheck)
{
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
