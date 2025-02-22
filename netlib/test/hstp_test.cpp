#include <QtCore/qcontainerfwd.h>
#include <QtCore/qstringview.h>
#include <cstddef>
#include <gtest/gtest.h>
#include <hstp.hpp>
#include <memory>

class HstpHandlerTest : public testing::Test
{
public:
  HstpHandler m_handler;

  HstpHandlerTest() {}
  ~HstpHandlerTest() override {}

  void SetUp() override {}

  void TearDown() override {}

  std::shared_ptr<QByteArray>
  _serialize(const std::shared_ptr<HSTP_Header> &hdr)
  {
    return m_handler._serialize(hdr);
  }
  std::shared_ptr<HSTP_Header>
  _deserialize(const std::shared_ptr<QByteArray> &buff)
  {
    return m_handler._deserialize(buff);
  }
};

TEST_F(HstpHandlerTest, SerializeTest_Basic)
{

  Option echo_option;
  std::shared_ptr<HSTP_Header> header = std::make_shared<HSTP_Header>();
  std::strcpy(header.get()->sender_alias, "Alex");
  echo_option.type = 0;
  echo_option.len = 6;
  echo_option.data = std::shared_ptr<char[]>(new char[echo_option.len]);
  const char *hello = "hello";
  std::strcpy(echo_option.data.get(), hello);
  echo_option.data[6] = 0;

  header->options.push_back(echo_option);

  std::shared_ptr<QByteArray> byte_array_ptr;
  byte_array_ptr = _serialize(header);

  EXPECT_EQ(byte_array_ptr->toHex(':').toStdString(),
            std::string("41:6c:65:78:00:00:00:00:00:00:00:00:00:00:00:00:00:"
                        "00:08:00:01:00:06:68:65:6c:6c:6f:00"));
}

TEST_F(HstpHandlerTest, SerializeTest_18CharAlias)
{
  Option echo_option;

  std::shared_ptr<HSTP_Header> header = std::make_shared<HSTP_Header>();

  strcpy(header->sender_alias, "AAAAAAAAAAAAAAAAAA");
  echo_option.type = 0;
  echo_option.len = 6;
  echo_option.data = std::shared_ptr<char[]>(new char[echo_option.len]);
  const char *hello = "hello";
  std::memcpy(echo_option.data.get(), hello, 6);

  header->options.push_back(echo_option);

  std::shared_ptr<QByteArray> byte_array = std::make_shared<QByteArray>();
  byte_array = _serialize(header);

  EXPECT_STREQ(
      byte_array->toHex(':').toStdString().c_str(),
      "41:41:41:41:41:41:41:41:41:41:41:41:41:41:41:41:41:41:08:00:01:00:06:"
      "68:65:6c:6c:6f:00");
}

TEST_F(HstpHandlerTest, DeserializeTest_Basic)
{
  Option echo_option1, echo_option2;
  std::shared_ptr<HSTP_Header> header = std::make_shared<HSTP_Header>();

  strcpy(header->sender_alias, "Alex");
  echo_option1.type = 0;
  echo_option1.len = 6;
  echo_option1.data = std::shared_ptr<char[]>(new char[echo_option1.len]);
  std::memcpy(echo_option1.data.get(), "hello", 6);

  echo_option2.type = 0;
  echo_option2.len = 6;
  echo_option2.data = std::shared_ptr<char[]>(new char[echo_option2.len]);
  std::memcpy(echo_option2.data.get(), "world", 6);
  header->options.push_back(echo_option1);
  header->options.push_back(echo_option2);

  std::shared_ptr<HSTP_Header> deserialized_header =
      _deserialize(_serialize(header));

  EXPECT_EQ(*header, *deserialized_header);
}

TEST_F(HstpHandlerTest, DeserializeTest_BasicFailure)
{
  Option echo_option1, echo_option2;
  std::shared_ptr<HSTP_Header> header1 = std::make_shared<HSTP_Header>();
  strcpy(header1->sender_alias, "Alex");
  echo_option1.type = 0;
  echo_option1.len = 6;
  echo_option1.data = std::shared_ptr<char[]>(new char[echo_option1.len]);
  std::memcpy(echo_option1.data.get(), "hello", 6);

  std::shared_ptr<HSTP_Header> header2 = std::make_shared<HSTP_Header>();
  strcpy(header2->sender_alias, "Alex");
  echo_option2.type = 0;
  echo_option2.len = 6;
  echo_option2.data = std::shared_ptr<char[]>(new char[echo_option2.len]);
  std::memcpy(echo_option2.data.get(), "world", 6);
  header2->options.push_back(echo_option1);
  header2->options.push_back(echo_option2);

  std::shared_ptr<HSTP_Header> deserialized_header1 =
      _deserialize(_serialize(header1));
  std::shared_ptr<HSTP_Header> deserialized_header2 =
      _deserialize(_serialize(header2));

  EXPECT_EQ(*header1, *deserialized_header1);
  EXPECT_EQ(*header2, *deserialized_header2);
  EXPECT_NE(*deserialized_header1, *deserialized_header2);
}
