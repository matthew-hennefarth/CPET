#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "Utilities.h"

TEST(Split, HandlesStandardInput) {
  EXPECT_EQ(
      cpet::util::split("This is a very hard case", ' '),
      std::vector<std::string>({"This", "is", "a", "very", "hard", "case"}));

  EXPECT_EQ(cpet::util::split("Try this for: a new: tokey", ':'),
            std::vector<std::string>({"Try this for", " a new", " tokey"}));
}

TEST(Split, EmptyParams) {
  EXPECT_EQ(cpet::util::split("", ' '), std::vector<std::string>({}))
      << "Does not return empty vector for string \"\"";

  EXPECT_EQ(cpet::util::split("", 'a'), std::vector<std::string>({}))
      << "Does not return empty vector for string \"\"";
  EXPECT_EQ(cpet::util::split("hello there world", '\0'),
            std::vector<std::string>({"hello there world"}))
      << "Fails when nullbyte deliminator";
}

TEST(isDouble, NumericInput) {
  EXPECT_PRED1(cpet::util::isDouble, "4");
  EXPECT_PRED1(cpet::util::isDouble, "4.");
  EXPECT_PRED1(cpet::util::isDouble, "4.0");
  EXPECT_PRED1(cpet::util::isDouble, ".0");
  EXPECT_PRED1(cpet::util::isDouble, "-23");
  EXPECT_PRED1(cpet::util::isDouble, "-23.0");
  EXPECT_PRED1(cpet::util::isDouble, "-.02");
  EXPECT_PRED1(cpet::util::isDouble, "0.1533458");
  EXPECT_PRED1(cpet::util::isDouble,
               "23897239840109230498120394881257918230471208347198235701298347."
               "123498134109283");
  EXPECT_PRED1(cpet::util::isDouble,
               "-10239841209381238971892370123940172034123412903471023984109234"
               "1.1298347128349123849128493");
  EXPECT_PRED1(cpet::util::isDouble, " 4.5");
  EXPECT_PRED1(cpet::util::isDouble, "    4.5");
  EXPECT_PRED1(cpet::util::isDouble, "4.5     ");
  EXPECT_PRED1(cpet::util::isDouble, "    4.6124    ");
}

bool isNotDouble(const std::string& str) { return !cpet::util::isDouble(str); }

TEST(isDouble, NoNumericInput) {
  EXPECT_PRED1(isNotDouble, "hellow");
  EXPECT_PRED1(isNotDouble, "Lets, try some; additional syntax");
  EXPECT_PRED1(isNotDouble, "^&*@(@(#&@(@)))");
  EXPECT_PRED1(isNotDouble, "<>:{}[]\\|");
  EXPECT_PRED1(isNotDouble, "\t \n");
}
TEST(isDouble, SomeNumericInput) {
  EXPECT_PRED1(isNotDouble, "a4.5");
  EXPECT_PRED1(isNotDouble, "4.12b");
  EXPECT_PRED1(isNotDouble, "4.5!2");
  EXPECT_PRED1(isNotDouble, ":-45.89");
  EXPECT_PRED1(isNotDouble, ">-45.65%");
  EXPECT_PRED1(isNotDouble, "4.5 6");
}
TEST(isDouble, SpaceInNumber) {
  EXPECT_PRED1(isNotDouble, "-12 .203");
  EXPECT_PRED1(isNotDouble, "- 19.1290");
  EXPECT_PRED1(isNotDouble, "123.  2390");
  EXPECT_PRED1(isNotDouble, "12 . 234");
}

TEST(find_if_ex, ValidElement) {
  std::vector<std::string> names = {"Bob", "mary", "john", "Jim"};
  std::vector<int> integers = {1, 3, 5, 2, 4, 6, 7, -10, 29, -56};
  EXPECT_EQ(cpet::util::find_if_ex(
                names.begin(), names.end(),
                [](const std::string& str) { return str == "mary"; }),
            std::find(names.begin(), names.end(), "mary"));

  EXPECT_EQ(cpet::util::find_if_ex(
                names.begin(), names.end(),
                [](const std::string& str) { return str == "john"; }),
            std::find(names.begin(), names.end(), "john"));

  EXPECT_EQ(cpet::util::find_if_ex(integers.begin(), integers.end(),
                                   [](int i) { return i == 3; }),
            std::find(integers.begin(), integers.end(), 3));

  EXPECT_EQ(cpet::util::find_if_ex(integers.begin(), integers.end(),
                                   [](int i) { return i == -10; }),
            std::find(integers.begin(), integers.end(), -10));
}

TEST(find_if_ex, ElementMissing) {
  std::vector<std::string> names = {"Bob", "mary", "john", "Jim"};
  std::vector<int> integers = {1, 3, 5, 2, 4, 6, 7, -10, 29, -56};

  EXPECT_THROW(cpet::util::find_if_ex(
                   names.begin(), names.end(),
                   [](const std::string& str) { return str == "John"; }),
               cpet::value_not_found);
  EXPECT_THROW(cpet::util::find_if_ex(
                   names.begin(), names.end(),
                   [](const std::string& str) { return str == "Brian"; }),
               cpet::value_not_found);

  EXPECT_THROW(cpet::util::find_if_ex(integers.begin(), integers.end(),
                                      [](int i) { return i == -1; }),
               cpet::value_not_found);
  EXPECT_THROW(cpet::util::find_if_ex(integers.begin(), integers.end(),
                                      [](int i) { return i == 32; }),
               cpet::value_not_found);
}

TEST(lstrip, NormalLstrips) {
  std::string str = "  the big grey fox was hungry";
  EXPECT_EQ(cpet::util::lstrip(str), "the big grey fox was hungry");
  EXPECT_EQ(cpet::util::lstrip(str, " \tt"), "he big grey fox was hungry");
  EXPECT_EQ(cpet::util::lstrip(str, "the"), str);

  str = "aaa;## hello there general kenobii    ";
  EXPECT_EQ(cpet::util::lstrip(str), str);
  EXPECT_EQ(cpet::util::lstrip(str, "a"),
            ";## hello there general kenobii    ");
  EXPECT_EQ(cpet::util::lstrip(str, "a;# "), "hello there general kenobii    ");
}

TEST(lstrip, EmptyString) {
  ASSERT_NO_THROW(auto s = cpet::util::lstrip(""));
  EXPECT_EQ(cpet::util::lstrip(""), "");
  EXPECT_EQ(cpet::util::lstrip("", "abcdefghijklmnopqrstuvwxyz1234567890"), "");
}

TEST(rstrip, NormalRstrip) {
  std::string str = "    the big grey fox was hungry";
  EXPECT_EQ(cpet::util::rstrip(str), str);
  EXPECT_EQ(cpet::util::rstrip(str, "yr"), "    the big grey fox was hung");
  EXPECT_EQ(cpet::util::rstrip(str, " \try"), "    the big grey fox was hung");

  str = "aaa;## hello there general kenobii;# my comment  ";

  EXPECT_EQ(cpet::util::rstrip(str),
            "aaa;## hello there general kenobii;# my comment");
  EXPECT_EQ(cpet::util::rstrip(str, "#"), str);
  EXPECT_EQ(cpet::util::rstrip(str, "t \tn"),
            "aaa;## hello there general kenobii;# my comme");
}

TEST(rstrip, EmptyString) {
  ASSERT_NO_THROW(auto s = cpet::util::rstrip(""));
  EXPECT_EQ(cpet::util::rstrip(""), "");
  EXPECT_EQ(cpet::util::rstrip("", "abcdefghijklmnopqrstuvwxyz1234567890"), "");
}

TEST(removeAfter, NormalRemoveAfter) {
  std::string str = "what is the big deal? #we place comments here...";

  EXPECT_EQ(cpet::util::removeAfter(str, "#"), "what is the big deal? ");
  EXPECT_EQ(cpet::util::removeAfter(str), "what");
  EXPECT_EQ(cpet::util::removeAfter(str, "p?"), "what is the big deal");
  EXPECT_EQ(cpet::util::removeAfter(str, "zy"), str);

  str = "   hello there general kenobi;#";
  EXPECT_EQ(cpet::util::removeAfter(str), "");
  EXPECT_EQ(cpet::util::removeAfter(str, "#"),
            "   hello there general kenobi;");
  EXPECT_EQ(cpet::util::removeAfter(str, "#;"),
            "   hello there general kenobi");
}

TEST(removeAfter, EmptyString) {
  ASSERT_NO_THROW(auto s = cpet::util::removeAfter(""));
  EXPECT_EQ(cpet::util::removeAfter(""), "");
  EXPECT_EQ(cpet::util::removeAfter("", "abcdefghijklmnopqrstuvwxyz1234567890"),
            "");
}

TEST(starts_with, NormalStartsWith) {
  std::string str = "I would like to greet you";

  EXPECT_TRUE(cpet::util::startswith(str, "I"));
  EXPECT_TRUE(cpet::util::startswith(str, str));
  EXPECT_TRUE(cpet::util::startswith(str, "I would like"));
  EXPECT_FALSE(cpet::util::startswith(str, "would"));
  EXPECT_FALSE(cpet::util::startswith(str, " would like to greet you"));
  EXPECT_FALSE(cpet::util::startswith(str, "I would like to greet you today"));

  str = "  %suprise";
  EXPECT_TRUE(cpet::util::startswith(str, "  "));
  EXPECT_TRUE(cpet::util::startswith(str, "  %s"));
  EXPECT_FALSE(cpet::util::startswith(str, "suprise"));
  EXPECT_FALSE(cpet::util::startswith(str, " %suprise"));
  EXPECT_FALSE(cpet::util::startswith(str, "  %suprise my dude"));
}

TEST(startswith, EmptyString) {
  ASSERT_NO_THROW(auto b = cpet::util::startswith("", "abcd"));
  EXPECT_FALSE(cpet::util::startswith("", "abcd"));
  EXPECT_TRUE(cpet::util::startswith("", ""));
  EXPECT_FALSE(cpet::util::startswith("", " "));

  ASSERT_NO_THROW(auto b = cpet::util::startswith("abcd", ""));
  EXPECT_TRUE(cpet::util::startswith("abcd", ""));
  EXPECT_TRUE(cpet::util::startswith("kenobi", ""));
  EXPECT_TRUE(cpet::util::startswith(" hello", ""));

  EXPECT_TRUE(cpet::util::startswith("", ""));
}
