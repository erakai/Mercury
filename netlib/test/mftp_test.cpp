#include "logger.hpp"
#include "mftp.hpp"
#include <QUdpSocket>
#include <chrono>
#include <gtest/gtest.h>

class MftpTest : public testing::Test
{
public:
  MftpTest()
  {
    MINIMUM_LOG_LEVEL = 1;

    sock1 = acquire_mftp_socket(32333);
    sock2 = acquire_mftp_socket(23222);

    // Create basic header to use later
    header.version = 1;
    header.version = 2;
    header.seq_num = 3;
    header.timestamp = 4;
    strcpy(header.source_name, "TesterName");
  }

  ~MftpTest()
  {
    MINIMUM_LOG_LEVEL = -1;

    sock1->close();
    sock2->close();
  }

  void spin_until_received(std::shared_ptr<QUdpSocket> socket, int timeout_ms)
  {
    auto start = std::chrono::system_clock::now();

    while (!socket->hasPendingDatagrams())
    {
      auto now = std::chrono::system_clock::now();
      auto elapsed =
          std::chrono::duration_cast<std::chrono::milliseconds>(now - start);

      ASSERT_LE(elapsed.count(), timeout_ms);
    }
  }

  std::shared_ptr<QUdpSocket> sock1;
  std::shared_ptr<QUdpSocket> sock2;
  MFTP_Header header;
};

TEST_F(MftpTest, SendAndProcessBasic)
{
  // Create structures
  const char *vc = "Test Video";
  QByteArray video(vc);
  const char *ac = "Test Audio";
  QByteArray audio(ac);

  header.audio_len = audio.size();
  header.video_len = video.size();
  AV_Frame frame = {audio, video};

  MFTP_Header received_header = {0};
  AV_Frame received_frame;

  // Send and receive
  ASSERT_TRUE(send_datagram(sock1, QHostAddress::LocalHost, sock2->localPort(),
                            header, frame));
  spin_until_received(sock2, 5000);
  ASSERT_TRUE(process_datagram(sock2, received_header, received_frame));

  EXPECT_EQ(received_header, header);
  EXPECT_EQ(received_frame.audio, frame.audio);
  EXPECT_EQ(received_frame.video, frame.video);
}
