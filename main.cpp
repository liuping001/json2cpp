
#include "json/json.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <algorithm>
#include <sstream>
#include <cstring>

using namespace std;

const char *kCommit = "/*********************************************************\n"
                      "*\n"
                      "* 文件自动生成. tool: https://github.com/liuping001/json2cpp\n"
                      "*\n"
                      "**********************************************************/\n\n";
const char *kInclude ="#include <cstdint>\n"
                      "#include <string>\n"
                      "#include <vector>\n"
                      "#include \"thirdparty/json/json.h\"\n\n";
const char *kTomlBase = "";
const char *kIfRootIsVector = "using VectorRoot = std::vector<Root>;\n"
                              "inline VectorRoot FromJson(Json::Value root) {\n"
                              "  VectorRoot ret;\n"
                              "  for (auto &item : root) {\n"
                              "    ret.emplace_back();\n"
                              "    ret.back().FromJson(item);\n"
                              "  }\n"
                              "  return ret;\n"
                              "};\n\n";
namespace json2cpp {

static std::string Tab;
static std::string toml_base_dir;

struct CppOut {
  std::ostringstream make_struct;
};

std::string Type(Json::Value root) {
  switch (root.type()) {
    case Json::ValueType::stringValue: return "std::string";
    case Json::ValueType::intValue: return "int64_t";
    case Json::ValueType::uintValue: return "uint64_t";
    case Json::ValueType::realValue: return "double";
    case Json::ValueType::booleanValue: return "bool";
  }
  return "error";
}

std::string GetAs(Json::Value root) {
  switch (root.type()) {
    case Json::ValueType::stringValue: return "asString()";
    case Json::ValueType::intValue: return "asInt64()";
    case Json::ValueType::uintValue: return "asUInt64()";
    case Json::ValueType::realValue: return "asDouble()";
    case Json::ValueType::booleanValue: return "asBool()";
  }
  return "error";
}

inline std::string BigWord(const std::string &org_word) {
  auto to_up = [](char c) -> char {
    if (c >= 'a' && c <= 'z') {
      return 'A' + (c - 'a');
    }
    return c;
  };

  auto word = org_word;
  for (size_t i = 0; i < word.size(); i++) {
    if (i==0 || (i > 0 && word[i - 1] == '_')) {
      word[i] = to_up(word[i]);
    }
  }
  word.erase(std::remove_if(word.begin(), word.end(), [](char c) { return c == '_';}), word.end());
  return word;
}

void StructDefineArray(Json::Value root, const std::string &key, CppOut &ss) {
  if (root.isArray()) {
    ss.make_struct << "std::vector<";
    StructDefineArray(root[0], key, ss);
    ss.make_struct << ">";
  } else if (root.isObject()) {
    ss.make_struct << BigWord(key);
  } else {
    ss.make_struct << Type(root);
  }
}

void MakeStructDefine(Json::Value root,
                      const std::string &key,
                      CppOut &ss,
                      const std::string &depth) {
  auto next_depth = depth + Tab;
  if (root.isArray()) {
    ss.make_struct << depth;
    StructDefineArray(root, key, ss);
    ss.make_struct << " " << key << ";\n";
  } else if (root.isObject()) {
    ss.make_struct << depth << BigWord(key) << " " << key << "; \n";
  } else {
    ss.make_struct << depth << Type(root) <<" " << key << ";\n";
  }
}

void MakeInitArray(Json::Value root, std::ostringstream &init_func, const std::string &depth,
                   const std::string &key, int32_t d) {
  std::string item1 = "arr_" + key + "_item" + std::to_string(d - 1);
  std::string item2 = "arr_" + key + "_item" + std::to_string(d);

  auto n_depth = depth + Tab;
  auto nn_depth = n_depth + Tab;
  auto first_item = root[0];
  if (first_item.isArray()) {
    init_func << n_depth << "for (auto item : arr_" << key << ") {\n";
    init_func << nn_depth << "auto arr_"<< key << " = item;\n";
    init_func << nn_depth << "decltype(" << item1 << ")::value_type " << item2 << ";\n";
    MakeInitArray(first_item, init_func, n_depth, key, d + 1);
    init_func << nn_depth << item1 << ".push_back(" << item2 << ");\n";
    init_func << n_depth << "}\n";
  } else if (first_item.isObject()) {
    init_func << n_depth << "for (auto item : arr_" << key <<") {\n";
    init_func << nn_depth << item1 << ".emplace_back();\n";
    init_func << nn_depth << item1 << ".back().FromJson(item);\n";
    init_func << n_depth << "}\n";
  } else {
    init_func << n_depth << "for (auto item : arr_" << key <<") {\n";
    init_func << nn_depth << item1 << ".push_back(item." << GetAs(first_item) << ");\n";
    init_func << n_depth << "}\n";
  }
}

void MakeInitFunc(Json::Value root,
                  const std::string &key,
                  std::ostringstream &init_func,
                  const std::string &depth) {
  auto n_depth = depth + Tab;
  if (root.isArray()) {
    auto d = 1;
    init_func << n_depth << "auto arr_" << key << " = root[\""<<key <<"\"];\n";
    init_func << n_depth << "decltype(" << key << ") arr_" << key << "_item" << d << ";\n";
    MakeInitArray(root, init_func, depth, key, d + 1);
    init_func << n_depth << key << " = " << "std::move(arr_" << key << "_item" << d << ");\n\n";
  } else if (root.isObject()) {
    init_func << n_depth << key << ".FromJson(root[\"" << key << "\"]);\n";
  } else {
    init_func << n_depth << key << " = root[\"" << key << "\"]."<< GetAs(root) <<";\n";
  }
}

// 由于依赖关系，所以先遍历到最低层数据结构 然后才分析结构
void PrintStruct(Json::Value root, const std::string &key, CppOut &ss, const std::string &depth) {
  auto n_depth = depth + Tab;
  auto nn_depth = n_depth + Tab;
  std::vector<std::pair<std::string, Json::Value>> depend;
  if (root.isObject()) {
    for (auto iter = root.begin(); iter != root.end(); iter++) {
      if (iter->isArray() || iter->isObject()) {
        depend.emplace_back(iter.key().asString(), *iter);
      }
    }
  } else if (root.isArray()) {
    depend.emplace_back(key, root[0]);
  }

  for (auto item : depend) {
    PrintStruct(item.second, item.first, ss, depth);
  }
  std::ostringstream init_func;
  if (root.isObject()) {
    ss.make_struct << "\n" << depth << "struct " << BigWord(key) << " {\n";
    init_func << "\n" << n_depth << "void FromJson(Json::Value root) {\n";
    for (auto iter = root.begin(); iter != root.end(); iter++) {
      MakeStructDefine(*iter, iter.key().asString(), ss, n_depth);
      MakeInitFunc(*iter, iter.key().asString(), init_func, n_depth);
    }
    init_func << n_depth << "}\n";
    ss.make_struct << init_func.str();
    ss.make_struct << depth << "};\n";
  }
}

} // end namespace json2cpp
const std::string basename(const std::string &path) {
  auto const pos = path.find_last_of('/');
  return pos == path.npos ? path : path.substr(pos + 1);
}

const std::string dirname(const std::string &path) {
  auto const pos = path.find_last_of('/');
  return pos == path.npos ? "" : path.substr(0, pos);
}


using namespace json2cpp;
int main(int argc, char *argv[]) {
  std::string file;
  Tab = "  ";
  if (argc < 3) {
    std::cout << "json2cpp -file file\n";
    return -1;
  }
  for (int i = 2; i < argc; i = i + 2) {
    if (strcmp(argv[i - 1], "-file") == 0) {
      file = argv[i];
    }
  }

  try {
    std::ifstream ifs(file);
    Json::Reader reader;
    Json::Value root;
    if ((!reader.parse(ifs, root, false))) {
      std::cerr << "reader.parse error\n";
      return -1;
    }

    auto file_name = std::string(basename(file));
    auto hpp = file_name.substr(0, file_name.find("."));

    CppOut out;
    PrintStruct(root, "root", out, "");

    std::ofstream fout(hpp + "_json2cpp.hpp", std::ios::out);

    fout << kCommit;
    fout << "#pragma once\n";
    fout << kInclude;
    fout << kTomlBase;
    fout << "namespace " << hpp << "_json2cpp {\n";
    fout << out.make_struct.str() << "\n";
    fout << kIfRootIsVector;
    fout << "} // end " << hpp << "_json2cpp \n";
    fout.close();
    std::cout << "finish to cpp:" << file_name << std::endl;
  } catch (std::exception e) {
    std::cout << "parse file failed :" << argv[1] << " e:"<<e.what();
  }
  return 0;
}