#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <string>
#include <sstream>

enum class Type {Name, Int, Float, String, Char};

typedef std::pair<Type, void*> Var;

std::map<std::string, Var> var_map {};

std::map<std::string, std::function<Var(std::vector<Var>)>> func_map {};

//std::map<std::string, std::function<Var()>> func_map {};

class Lisp_error {
public:
  Lisp_error(const std::string& n_message): message {n_message} {}
  std::string what() { return message; }
private:
  std::string message;
};

class Argument_error : public Lisp_error {
public:
  using Lisp_error::Lisp_error;
};

class Type_error : public Lisp_error {
public:
  using Lisp_error::Lisp_error;
};

Var set(std::vector<Var> arg_vec);

Var print(std::vector<Var> arg_vex);

Var create_var(const std::string& str_rep);

Var eval(const std::string& expr_str);

int main(int argc, char* argv[]) {
  func_map.insert({"set", set});
  func_map.insert({"print", print});
  std::cout << "Input expresions to evaluate:\n";
  std::string expr;
  while (std::getline(std::cin, expr)) {
    try {
      eval(expr);
      std::cout << std::endl;
    } catch (Lisp_error& e) {
      std::cerr << "error: " << e.what() << "\n";
      return -1;
    }
  }
}

Var set(std::vector<Var> arg_vec)
{
  if (arg_vec.size() != 2) throw Argument_error {"'set' called with an incorrect number of arguments"};
  Var name = arg_vec[0];
  Var contents = arg_vec[1];
  if (name.first != Type::Name)
    throw Type_error {"'set' called with an incorrect type for first variable"};
  var_map[*static_cast<std::string*>(name.second)] = contents;
  return contents;
}

Var print(std::vector<Var> arg_vec)
{
  if (arg_vec.size() != 1) throw Argument_error {"'print' called with an incorrect number of arguments"};
  Var variable = arg_vec[0];
  switch (variable.first) {
  case Type::Int:
    std::cout << *static_cast<int*>(variable.second);
    break;
  case Type::Float:
    std::cout << *static_cast<double*>(variable.second);
  case Type::Char:
    std::cout << *static_cast<char*>(variable.second);
  case Type::String: case Type::Name:
    std::cout << *static_cast<std::string*>(variable.second);
    break;
  default:
    throw Type_error {"'print' called with an incorrect type"};
    break;
  }
  return variable;
}

Var create_var(const std::string& str_rep)
{
  // First check for 'Type::Name'
  std::regex type_regex {R"('(.+))"};
  std::smatch matches {};
  if (std::regex_match(str_rep, matches, type_regex)) {
    return Var {Type::Name, static_cast<void*>(new std::string {matches[1]})};
  }
  type_regex = R"**("(.+)")**";
  if (std::regex_match(str_rep, matches, type_regex)) {
    return Var {Type::String, static_cast<void*>(new std::string {matches[1]})};
  }
  type_regex = R"('(.)')";
  if (std::regex_match(str_rep, matches, type_regex)) {
    return Var {Type::String, static_cast<char*>(new char {matches[1][0]})};
  }
  type_regex = R"(([[:digit:]]+))";
  if (std::regex_match(str_rep, matches, type_regex)) {
    return Var {Type::Int, static_cast<void*>(new int {std::stoi(matches[1])})};
  }
  type_regex = R"(([[:digit:]]*\.[[:digit:]]*))";
  if (std::regex_match(str_rep, matches, type_regex)) {
    return Var {Type::Double, static_cast<void*>(new double {std::stoi(matches[1])})};
  }
  auto var = var_map.find(str_rep);
  if (var == var_map.end())
    throw Argument_error {"create_var: nonexistant variable"};
  return var->second;
}

Var eval(const std::string& expr_str)
{
  std::istringstream expr {expr_str};
  Var ret;
  while (expr.get() != '(') {if (!expr) throw Lisp_error {"eval: no first parathesis"}; }
  std::string func_name;
  expr >> func_name;
  auto func = func_map.find(func_name);
  if (func_name == "setq")
    func = func_map.find("set");
  if (func == func_map.end()) throw Lisp_error {"eval: nonexistant function"};
  
  std::vector<Var> arguments {};
  std::string arg_str {};
  
  // while the stream is still valid, read until the end parenthesis
  char c;
  while (expr.get(c) && c != ')') {
    //std::cout << c << std::flush;
    if (c == ' ') {
      if (arg_str != "") {
	if (func_name == "setq")
	  arg_str = "'" + arg_str;
	arguments.push_back(create_var(arg_str));
	arg_str = "";
      }
    } else if (c == '(') {
      std::string portion {c};
      int depth = 1;
      while (depth) {
	expr.get(c);
	portion += c;
	if (c == '(')
	  ++depth;
	else if (c == ')')
	  --depth;
      }
      arguments.push_back(eval(portion));
    } else {
      arg_str += c;
    }
  }
  if (arg_str != "") {
    arguments.push_back(create_var(arg_str));
    arg_str = "";
  }
  if (c != ')') throw Lisp_error {"eval: unmatched parenthesis"};
  return func->second(arguments);
}
