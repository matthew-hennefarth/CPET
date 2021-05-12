#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

#include "Exceptions.h"
#include "Utilities.h"

TEST(Split, HandlesStandardInput) {
  EXPECT_EQ(
      split("This is a very hard case", ' '),
      std::vector<std::string>({"This", "is", "a", "very", "hard", "case"}));

  EXPECT_EQ(split("Try this for: a new: tokey", ':'),
            std::vector<std::string>({"Try this for", " a new", " tokey"}));
}

TEST(Split, EmptyParams) {
  EXPECT_EQ(split("", ' '), std::vector<std::string>({}))
      << "Does not return empty vector for string \"\"";

  EXPECT_EQ(split("", 'a'), std::vector<std::string>({}))
      << "Does not return empty vector for string \"\"";
  EXPECT_EQ(split("hello there world", '\0'),
            std::vector<std::string>({"hello there world"}))
      << "Fails when nullbyte deliminator";
}

TEST(isDouble, NumericInput) {
  EXPECT_PRED1(isDouble, "4");
  EXPECT_PRED1(isDouble, "4.");
  EXPECT_PRED1(isDouble, "4.0");
  EXPECT_PRED1(isDouble, ".0");
  EXPECT_PRED1(isDouble, "-23");
  EXPECT_PRED1(isDouble, "-23.0");
  EXPECT_PRED1(isDouble, "-.02");
  EXPECT_PRED1(isDouble, "0.1533458");
  EXPECT_PRED1(isDouble,
               "23897239840109230498120394881257918230471208347198235701298347."
               "123498134109283");
  EXPECT_PRED1(isDouble,
               "-10239841209381238971892370123940172034123412903471023984109234"
               "1.1298347128349123849128493");
  EXPECT_PRED1(isDouble, " 4.5");
  EXPECT_PRED1(isDouble, "    4.5");
  EXPECT_PRED1(isDouble, "4.5     ");
  EXPECT_PRED1(isDouble, "    4.6124    ");
}

bool isNotDouble(const std::string& str) { return !isDouble(str); }

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
  EXPECT_EQ(find_if_ex(names.begin(), names.end(),
                       [](const std::string& str) { return str == "mary"; }),
            std::find(names.begin(), names.end(), "mary"));

  EXPECT_EQ(find_if_ex(names.begin(), names.end(),
                       [](const std::string& str) { return str == "john"; }),
            std::find(names.begin(), names.end(), "john"));

  EXPECT_EQ(find_if_ex(integers.begin(), integers.end(),
                       [](int i) { return i == 3; }),
            std::find(integers.begin(), integers.end(), 3));

  EXPECT_EQ(find_if_ex(integers.begin(), integers.end(),
                       [](int i) { return i == -10; }),
            std::find(integers.begin(), integers.end(), -10));
}

TEST(find_if_ex, ElementMissing) {
  std::vector<std::string> names = {"Bob", "mary", "john", "Jim"};
  std::vector<int> integers = {1, 3, 5, 2, 4, 6, 7, -10, 29, -56};

  EXPECT_THROW(find_if_ex(names.begin(), names.end(),
                          [](const std::string& str) { return str == "John"; }),
               cpet::value_not_found);
  EXPECT_THROW(
      find_if_ex(names.begin(), names.end(),
                 [](const std::string& str) { return str == "Brian"; }),
      cpet::value_not_found);

  EXPECT_THROW(find_if_ex(integers.begin(), integers.end(),
                          [](int i) { return i == -1; }),
               cpet::value_not_found);
  EXPECT_THROW(find_if_ex(integers.begin(), integers.end(),
                          [](int i) { return i == 32; }),
               cpet::value_not_found);
}

