#include <QtCore/qstringview.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <hstp.hpp>

class HstpHandlerTester : public testing::Test
{
public:
  HstpHandler m_handler;

  HstpHandlerTester() {}
  ~HstpHandlerTester() override {}

  void SetUp() override {}

  void TearDown() override {}

  QByteArray _serialize(HSTP_Header &hdr) { return m_handler._serialize(hdr); }
};

TEST_F(HstpHandlerTester, SerializeTest1)
{
  Option echo_option;

  HSTP_Header header;
  strcpy(header.sender_alias, "Alex");
  echo_option.type = 1;
  echo_option.len = 7;
  echo_option.data = "hello";

  header.options.push_back(echo_option);

  QByteArray byte_array;
  byte_array = _serialize(header);

  EXPECT_STREQ(byte_array.toHex(':').toStdString().c_str(),
               "41:6c:65:78:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01:01:07:"
               "68:65:6c:6c:6f");
}

TEST_F(HstpHandlerTester, SerializeTest2)
{
  Option echo_option;

  HSTP_Header header;
  strcpy(header.sender_alias, "AAAAAAAAAAAAAAAAAA");
  echo_option.type = 1;
  echo_option.len = 7;
  echo_option.data = "hello";

  header.options.push_back(echo_option);

  QByteArray byte_array;
  byte_array = _serialize(header);

  EXPECT_STREQ(byte_array.toHex(':').toStdString().c_str(),
               "41:41:41:41:41:41:41:41:41:41:41:41:41:41:41:41:41:41:01:01:07:"
               "68:65:6c:6c:6f");
}
