#include "logger.hpp"
#include "mftp.hpp"
#include <QApplication>
#include <QAudioBuffer>
#include <QBuffer>
#include <QImage>
#include <QPixmap>
#include <QScreen>
#include <QUdpSocket>
#include <chrono>
#include <gtest/gtest.h>

class MftpTest : public testing::Test
{
public:
  MftpTest()
  {
    MINIMUM_LOG_LEVEL = 1;

    int argc = 0;
    char *argv[] = {nullptr};
    app = new QGuiApplication(argc, argv);

    sock1 = acquire_mftp_socket(32333);
    sock2 = acquire_mftp_socket(23222);
    sock2_processor = std::make_shared<MFTPProcessor>();

    // Create basic header to use later
    sent_header.version = 1;
    sent_header.payload_type = 2;
    sent_header.seq_num = 1;
    sent_header.timestamp = 4;
    strcpy(sent_header.source_name, "TesterName");
  }
  ~MftpTest()
  {
    MINIMUM_LOG_LEVEL = -1;

    sock1->close();
    sock2->close();

    delete app;
  }

  std::shared_ptr<QUdpSocket> sock1;
  std::shared_ptr<QUdpSocket> sock2;
  std::shared_ptr<MFTPProcessor> sock2_processor;
  MFTP_Header sent_header;
  QGuiApplication *app;
};

TEST_F(MftpTest, SendAndProcessBasic)
{

  // Create data
  QAudioBuffer sent_audio;

  QPixmap video = QGuiApplication::primaryScreen()->grabWindow(0);
  QImage sent_image = video.toImage();

  // Set up variables we will fill
  MFTP_Header received_header = {0};
  QImage received_image;
  QAudioBuffer received_audio;

  // Connect ready read to our processor
  QObject::connect(sock2.get(), &QUdpSocket::readyRead,
                   [&]() { sock2_processor->process_ready_datagrams(sock2); });

  // Connect our processor to receiving the data
  QObject::connect(sock2_processor.get(), &MFTPProcessor::frame_ready,
                   [&](MFTP_Header header, QAudioBuffer audio, QImage video)
                   {
                     received_header = header;
                     received_image = video;
                     received_audio = audio;
                   });

  // Send and receive
  auto before_send = std::chrono::system_clock::now();
  ASSERT_TRUE(send_datagram(sock1, QHostAddress::LocalHost, sock2->localPort(),
                            sent_header, sent_image, sent_audio));
  auto after_send = std::chrono::system_clock::now();
  auto send_time = std::chrono::duration_cast<std::chrono::milliseconds>(
      after_send - before_send);
  std::cout << "\u001b[32m[          ] \u001b[33m"
            << std::format("Elapsed (Sending 1 frame): {} ms",
                           send_time.count())
            << "\u001b[0m\n"
            << std::flush;

  // Spin until timeout or we receive frame
  auto start = std::chrono::system_clock::now();
  constexpr int timeout_ms = 5000;

  while (received_header.seq_num == 0)
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
            << std::format("Elapsed (Receiving 1 frame): {} ms",
                           elapsed.count())
            << "\u001b[0m\n"
            << std::flush;

  // This will vary
  sent_header.fragment_num = received_header.fragment_num;

  // Check audio and header
  ASSERT_EQ(sent_header, received_header);
  EXPECT_EQ(sent_audio.constData<char>(), received_audio.constData<char>());

  // Correctly removed it
  EXPECT_EQ(sock2_processor->partial_frames[0].remaining_fragments, -1);

  // Check size of vide odata
  QByteArray sent_video_bytes;
  QBuffer sent_video_buffer(&sent_video_bytes);
  sent_video_buffer.open(QIODevice::WriteOnly);
  sent_image.save(&sent_video_buffer, "JPG");
  sent_video_buffer.close();

  QByteArray received_video_bytes;
  QBuffer received_video_buffer(&received_video_bytes);
  received_video_buffer.open(QIODevice::WriteOnly);
  ASSERT_TRUE(received_image.save(&received_video_buffer, "JPG"));
  received_video_buffer.close();

  // Size can vary based on how it serializes
  ASSERT_NEAR(sent_video_bytes.size(), received_video_bytes.size(), 1500);

  // Examine this in: build/assets/...
  ASSERT_TRUE(received_image.save("assets/test.jpg", "JPG"));
}
