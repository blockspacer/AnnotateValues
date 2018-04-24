//
//
//

#include "Stats.hpp"

#include "rapidjson/document.h"
// using rapidjson::Document

#include "rapidjson/ostreamwrapper.h"
// using rapidjson::OStreamWrapper

#include "rapidjson/prettywriter.h"
// using rapidjson::PrettyWriter

#include <fstream>
// using std::ofstream

namespace json = rapidjson;

namespace icsa {

bool AnnotateInstructionsStats::save(const std::string &Filename) const {
  json::Document d;
  d.SetObject();

  auto &allocator = d.GetAllocator();

  d.AddMember("functions_processed", Functions.size(), allocator);

  json::Value functions(json::kArrayType);
  for (const auto &e : Functions) {
    json::Value s{e.c_str(), allocator};
    functions.PushBack(s, allocator);
  }

  d.AddMember("functions", functions, allocator);

  std::ofstream ofs(Filename);
  json::OStreamWrapper osw(ofs);
  json::PrettyWriter<decltype(osw)> writer(osw);
  writer.SetIndent(' ', 2);

  d.Accept(writer);

  return true;
}

} // namespace icsa
