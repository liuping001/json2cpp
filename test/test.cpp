//
// Created by mico on 2020/6/11.
//

#include <fstream>
#include "json/json.h"
#include "test_json2cpp.hpp"
#include "test_base.h"


TEST_F(JsonToCpp) {
  std::ifstream ifs("../test/test.json");
  Json::Reader reader;
  Json::Value root;
  if ((!reader.parse(ifs, root, false))) {
    std::cerr << "reader.parse error\n";
    ASSERT(false);
  }

  auto data = test_json2cpp::FromJson(root);

  ASSERT_EQ(data[0].db_id, 194);
  ASSERT_EQ(data[0].config.json_schema,"family_region_info");
  ASSERT_EQ(data[0].config.family_region_info.size(), 8);
  ASSERT_EQ(data[0].config.family_region_info[7].region_ar, false);
  ASSERT_EQ(data[0].config.family_region_info[7].region_tr, true);
  ASSERT_EQ(data[0].config.family_region_info[7].grade1, "10000,15000,20000,30000,50000");
}

TEST_FINSH