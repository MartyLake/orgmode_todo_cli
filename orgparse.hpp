#pragma once
#include <istream>
#include <string>
#include <vector>

// tokenizer
struct Token {
  enum Type { literal, tree_node_nesting_value, todo, next, done } type;
  std::string str_value;
  size_t int_value;
  Token(Type t, std::string s, size_t i);
  inline Token(){};
  bool operator==(const Token &other) const;
  bool operator!=(const Token &other) const;
};
std::vector<Token> tokenize(std::istream &s);
std::ostream &operator<<(std::ostream &os, Token::Type const &value);
std::ostream &operator<<(std::ostream &os, Token const &value);
