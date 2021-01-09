#include "string_utils.hpp"
#include <catch.hpp>

TEST_CASE("ltrim", "[string_utils]") {
  std::string s{"  toto   "};
  utils::ltrim(s);
  REQUIRE(s == "toto   ");
}
TEST_CASE("rtrim", "[string_utils]") {
  std::string s{"  toto   "};
  utils::rtrim(s);
  REQUIRE(s == "  toto");
}
TEST_CASE("trim", "[string_utils]") {
  std::string s{"  toto   "};
  utils::trim(s);
  REQUIRE(s == "toto");
}
TEST_CASE("replace_all", "[string_utils]") {
  std::string s{"one 2 three 2 4"};
  utils::replace_all(s, {"2", "two"});
  REQUIRE(s == "one two three two 4");
}
