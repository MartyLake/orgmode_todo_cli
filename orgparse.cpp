#include "orgparse.hpp"
#include <tuple>

using namespace std;
bool Token::operator==(const Token &other) const {
  return make_tuple(type, str_value, int_value) ==
         make_tuple(other.type, other.str_value, other.int_value);
}

std::ostream &operator<<(std::ostream &os, Token::Type const &value) {
  switch (value) {
  case Token::Type::literal:
    os << "literal";
    break;
  case Token::Type::tree_node_nesting_value:
    os << "level";
    break;
  case Token::Type::todo:
    os << "todo";
    break;
  case Token::Type::next:
    os << "next";
    break;
  case Token::Type::done:
    os << "done";
    break;
  }
  return os;
}
std::ostream &operator<<(std::ostream &os, Token const &value) {
  //    os << convertMyTypeToString( value );
  os << "type:" << value.type << " _str:" << value.str_value
     << " _value:" << value.int_value;
  return os;
}
vector<Token> tokenize(istream &s) {
  vector<Token> tokens;
  string line;
  while (getline(s, line)) {
    if (auto it =
            find_if(line.begin(), line.end(), [](char c) { return c != '*'; });
        it != line.begin()) {
      const string before(line.begin(), it);
      const size_t nest_level = count(before.begin(), before.end(), '*');
      tokens.emplace_back(Token::tree_node_nesting_value, before, nest_level);
      if (it == line.end())
        continue;
      ++it;
      if (it == line.end())
        continue;
      const string after(it, line.end());
      line = after;
    }
    const auto extract_status_token = [&](const string &token_str,
                                          Token::Type type) {
      if (line.find(token_str) == 0) {
        tokens.emplace_back(type, string{}, 0);
        return true;
      }
      return false;
    };
    bool has_status = extract_status_token("TODO", Token::todo);
    has_status |= extract_status_token("NEXT", Token::next);
    has_status |= extract_status_token("DONE", Token::done);
    if (has_status) {
      const auto after_begin = next(line.begin(), 5);
      if (after_begin < line.end()) {
        const string after(after_begin, line.end());
        line = after;
      } else {
        line = "";
      }
    }
    if (!line.empty()) {
      tokens.emplace_back(Token::literal, line, 0);
    }
  }
  return tokens;
}
