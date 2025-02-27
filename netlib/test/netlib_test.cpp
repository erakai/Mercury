#include "mercury_client.hpp"
#include "mercury_server.hpp"
#include <QApplication>
#include <QBuffer>
#include <QScreen>
#include <gtest/gtest.h>
#include <random>
#include <thread>

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
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  app->processEvents(QEventLoop::AllEvents, QDeadlineTimer(100));
  ASSERT_EQ(server.get_clients().size(), 1);

  // Test validation
  int client_id = (server.get_clients().begin())->second.id;
  EXPECT_EQ(server.get_client(client_id).validated, true);
  EXPECT_EQ(server.get_client(client_id).alias, client.get_alias());

  // Test MFTP
  QAudioBuffer audio;

  // Get screen for test video data
  QPixmap video = QGuiApplication::primaryScreen()->grabWindow(0);
  QImage image = video.toImage();
  QVideoFrame video_frame(image);

  // Need to also test sending a bunch of frames out of order (in a different
  // test case) to make sure client reorders them
  ASSERT_EQ(server.send_frame("test", audio, video_frame), 1);

  // Wait until full frame arrives
  auto start = std::chrono::system_clock::now();
  constexpr int timeout_ms = 2000;

  while (client.jitter_buffer_size() == 0)
  {
    auto now = std::chrono::system_clock::now();
    auto elapsed =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

    app->processEvents(QEventLoop::AllEvents, QDeadlineTimer(100));

    ASSERT_LE(elapsed.count(), timeout_ms);
  }

  auto now = std::chrono::system_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
  std::cout << "\u001b[32m[          ] \u001b[33m"
            << std::format("Elapsed (1 frame): {} ms", elapsed.count())
            << "\u001b[0m\n"
            << std::flush;

  JitterEntry frame = client.retrieve_next_frame();

  // Retrieved valid frame
  ASSERT_NE(frame.seq_num, -1);

  QByteArray sent_video_bytes;
  QBuffer sent_video_buffer(&sent_video_bytes);
  sent_video_buffer.open(QIODevice::WriteOnly);
  image.save(&sent_video_buffer, "JPG");
  sent_video_buffer.close();

  QByteArray received_video_bytes;
  QBuffer received_video_buffer(&received_video_bytes);
  received_video_buffer.open(QIODevice::WriteOnly);
  ASSERT_TRUE(frame.video.save(&received_video_buffer, "JPG"));
  received_video_buffer.close();

  EXPECT_EQ(frame.seq_num, 1);
  EXPECT_NE(frame.timestamp, 0);
  EXPECT_NEAR(sent_video_buffer.size(), received_video_bytes.size(), 1500);
}

// Demonstrate some basic assertions, sanity check.
TEST(HelloWorldTest, SanityCheck)
{
  // Expect two strings not to be equal.
  EXPECT_STRNE("hello", "world");
  // Expect equality.
  EXPECT_EQ(7 * 6, 42);
}
