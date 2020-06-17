/*********************************************************
*
* 文件自动生成. tool: https://github.com/liuping001/json2cpp
*
**********************************************************/

#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "json/json.h"

namespace test_json2cpp {

struct FamilyRegionInfo {
  int64_t create_family_level;
  std::string grade1;
  std::string grade2;
  std::string grade3;
  std::string grade4;
  std::string grade5;
  bool region_ar;
  bool region_id;
  bool region_in;
  bool region_th;
  bool region_tr;
  bool region_tw;
  bool region_us;
  bool region_vn;

  void FromJson(Json::Value root) {
    create_family_level = root["create_family_level"].asInt64();
    grade1 = root["grade1"].asString();
    grade2 = root["grade2"].asString();
    grade3 = root["grade3"].asString();
    grade4 = root["grade4"].asString();
    grade5 = root["grade5"].asString();
    region_ar = root["region_ar"].asBool();
    region_id = root["region_id"].asBool();
    region_in = root["region_in"].asBool();
    region_th = root["region_th"].asBool();
    region_tr = root["region_tr"].asBool();
    region_tw = root["region_tw"].asBool();
    region_us = root["region_us"].asBool();
    region_vn = root["region_vn"].asBool();
  }
};

struct Config {
  std::vector<FamilyRegionInfo> family_region_info;
  std::string json_schema;

  void FromJson(Json::Value root) {
    auto arr_family_region_info = root["family_region_info"];
    decltype(family_region_info) arr_family_region_info_item1;
    for (auto item : arr_family_region_info) {
      arr_family_region_info_item1.emplace_back();
      arr_family_region_info_item1.back().FromJson(item);
    }
    family_region_info = std::move(arr_family_region_info_item1);

    json_schema = root["json_schema"].asString();
  }
};

struct Test {
  std::vector<int64_t> test;

  void FromJson(Json::Value root) {
    auto arr_test = root["test"];
    decltype(test) arr_test_item1;
    for (auto item : arr_test) {
      arr_test_item1.push_back(item.asInt64());
    }
    test = std::move(arr_test_item1);

  }
};

struct Root {
  Config config; 
  std::string country;
  int64_t db_id;
  int64_t support_device;
  std::vector<std::vector<std::vector<Test>>> test;

  void FromJson(Json::Value root) {
    config.FromJson(root["config"]);
    country = root["country"].asString();
    db_id = root["db_id"].asInt64();
    support_device = root["support_device"].asInt64();
    auto arr_test = root["test"];
    decltype(test) arr_test_item1;
    for (auto item : arr_test) {
      auto arr_test = item;
      decltype(arr_test_item1)::value_type arr_test_item2;
      for (auto item : arr_test) {
        auto arr_test = item;
        decltype(arr_test_item2)::value_type arr_test_item3;
        for (auto item : arr_test) {
          arr_test_item3.emplace_back();
          arr_test_item3.back().FromJson(item);
        }
        arr_test_item2.push_back(arr_test_item3);
      }
      arr_test_item1.push_back(arr_test_item2);
    }
    test = std::move(arr_test_item1);

  }
};

using VectorRoot = std::vector<Root>;
inline VectorRoot FromJson(Json::Value root) {
  VectorRoot ret;
  for (auto &item : root) {
    ret.emplace_back();
    ret.back().FromJson(item);
  }
  return ret;
};

} // end test_json2cpp 
