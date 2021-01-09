#include "orgparse.hpp"
#include <catch.hpp>
#include <sstream>

using namespace std;
using Catch::Matchers::Equals;

const auto tokenize_string = [](string s) {
  stringstream stream(s);
  return tokenize(stream);
};
TEST_CASE("tokenize simple token", "[orgparse]") {
  auto t = tokenize_string;
  REQUIRE_THAT(t("coucou toto"),
               Equals(vector<Token>{{Token::literal, "coucou toto", 0}}));
  REQUIRE_THAT(t("**** TEST"), Equals(vector<Token>{
                                {Token::tree_node_nesting_value, "****", 4},
                                {Token::literal, "TEST", 0},
                            }));
  REQUIRE_THAT(t("* TODO"), Equals(vector<Token>{
                                {Token::tree_node_nesting_value, "*", 1},
                                {Token::todo, "", 0},
                            }));
  REQUIRE_THAT(t("* NEXT"), Equals(vector<Token>{
                                {Token::tree_node_nesting_value, "*", 1},
                                {Token::next, "", 0},
                            }));
  REQUIRE_THAT(t("* DONE"), Equals(vector<Token>{
                                {Token::tree_node_nesting_value, "*", 1},
                                {Token::done, "", 0},
                            }));
}
TEST_CASE("tokenize string with multiple token TODO", "[orgparse]") {
  auto t = tokenize_string;
  REQUIRE_THAT(t("* TODO NEXT"), Equals(vector<Token>{
                                     {Token::tree_node_nesting_value, "*", 1},
                                     {Token::todo, "", 0},
                                     {Token::literal, "NEXT", 0},
                                 }));
}
