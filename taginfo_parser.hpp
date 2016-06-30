#ifndef TAGINFO_VALIDATE_TAGINFO_PARSER
#define TAGINFO_VALIDATE_TAGINFO_PARSER

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

#define RAPIDJSON_ASSERT(x)                                                                                                      \
  if (!static_cast<bool>(x))                                                                                                     \
    throw std::runtime_error{#x};

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "tag.hpp"

namespace taginfo_validate {

// Creates a typed and efficient queryable in-memory database from a taginfo.json file.
//
//  - taginfo_parser(file):
//      constructor builds the database
//
//  - tags_on_nodes()
//      returns iterator pair for tags only allowed on nodes
//
//  - tags_on_ways()
//      returns iterator pair for tags only allowed on ways
//
//  - tags_on_relations()
//      returns iterator pair for tags only allowed on relations
//
//  - tags_on_areas()
//      returns iterator pair for tags only allowed on areas
//
//  - tags_on_any_object()
//      returns iterator pair for tags allowed on nodes, ways, relations, areas
//      note: the functions above do not contain items from this catch-all range
//
struct taginfo_parser {
  static const constexpr auto data_format = 1;

  explicit taginfo_parser(const boost::filesystem::path &taginfo) {
    std::ifstream taginfo_file{taginfo.string()};

    if (!taginfo_file)
      throw std::runtime_error{"unable to open taginfo file"};

    rapidjson::IStreamWrapper taginfo_stream{taginfo_file};

    rapidjson::Document json;
    json.ParseStream(taginfo_stream);

    if (json.HasParseError())
      throw std::runtime_error{"unable to parse taginfo file"};

    if (json["data_format"].GetInt() != data_format)
      throw std::runtime_error{"taginfo data format v1 supported only"};

    const auto &json_tags = json["tags"];

    tags.resize(json_tags.Size());

    // key, value (optional), type (optional)
    std::transform(json_tags.Begin(), json_tags.End(), begin(tags), [](const rapidjson::Value &json_tag) {
      const auto *key = json_tag["key"].GetString();

      const auto *value = [&] {
        const auto it = json_tag.FindMember("value");
        return (it != json_tag.MemberEnd()) ? it->value.GetString() : "";
      }();

      auto type = [&] {
        const auto it = json_tag.FindMember("object_types");

        if (it == json_tag.MemberEnd()) {
          return object::type::all;
        } else {
          const auto &types = it->value.GetArray();

          auto rv = object::type::unknown;

          for (auto it = types.Begin(), end = types.End(); it != end; ++it) {
            const auto *type_str = it->GetString();

            if (!std::strcmp(type_str, "node"))
              rv = object::type((unsigned)rv | object::type::node);
            else if (!std::strcmp(type_str, "way"))
              rv = object::type((unsigned)rv | object::type::way);
            else if (!std::strcmp(type_str, "relation"))
              rv = object::type((unsigned)rv | object::type::relation);
            else if (!std::strcmp(type_str, "area"))
              rv = object::type((unsigned)rv | object::type::area);
            else
              throw std::runtime_error{"taginfo contains unsupported object type"};
          }

          return rv;
        }
      }();

      return tag{key, value, type};
    });

    std::sort(begin(tags), end(tags));
  }

  std::vector<tag> tags;

  using tag_iter = decltype(tags)::const_iterator;

  std::pair<tag_iter, tag_iter> tags_on_nodes() const {
    return std::equal_range(begin(tags), end(tags), tag{{}, {}, object::type::node});
  }

  std::pair<tag_iter, tag_iter> tags_on_ways() const {
    return std::equal_range(begin(tags), end(tags), tag{{}, {}, object::type::way});
  }

  std::pair<tag_iter, tag_iter> tags_on_relations() const {
    return std::equal_range(begin(tags), end(tags), tag{{}, {}, object::type::relation});
  }

  std::pair<tag_iter, tag_iter> tags_on_areas() const {
    return std::equal_range(begin(tags), end(tags), tag{{}, {}, object::type::area});
  }

  std::pair<tag_iter, tag_iter> tags_on_any_object() const {
    return std::equal_range(begin(tags), end(tags), tag{{}, {}, object::type::all});
  }
};
}

#endif
