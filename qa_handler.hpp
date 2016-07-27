#ifndef TAGINFO_VALIDATE_QA_HANDLER
#define TAGINFO_VALIDATE_QA_HANDLER

#include <algorithm>
#include <osmium/handler.hpp>
#include <osmium/osm/types.hpp>
#include <set>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "tag.hpp"
#include "taginfo_parser.hpp"

// the handler checks if osm objects of any type are present in a given taginfo file

using namespace taginfo_validate;

struct qa_handler : osmium::handler::Handler {
  qa_handler(const taginfo_parser &taginfo)
      : tags_on_areas(taginfo.tags_on_areas()), tags_on_nodes(taginfo.tags_on_nodes()),
        tags_on_ways(taginfo.tags_on_ways()), tags_on_relations(taginfo.tags_on_relations()),
        tags_on_any_object(taginfo.tags_on_any_object()){};

  using tag_range = std::pair<taginfo_parser::tag_iter, taginfo_parser::tag_iter>;
  struct keyCompare {
    bool operator()(const tag &left, const tag &right) { return left.key < right.key; }
  };

  // if pbf tag is not found in taginfo list, store tag to list of unrecognized tags
  void verifyAndStoreTags(const object::type &objectType, const tag_range &tagRange, const osmium::Tag &thePbfTag) {
    // Look for the given pbf tag's key in the range of acceptable keys
    auto matching_key_tags = std::equal_range(tagRange.first, tagRange.second, tag{thePbfTag.key(), {}, objectType}, keyCompare());
    auto matching_key_tags_anyObject = std::equal_range(tags_on_any_object.first, tags_on_any_object.second,
                                                        tag{thePbfTag.key(), {}, object::type::all}, keyCompare());

    if (std::distance(matching_key_tags_anyObject.first, matching_key_tags_anyObject.second) == 0 &&
        std::distance(matching_key_tags.first, matching_key_tags.second) == 0) {
      unknown_types.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
      return;
    }

    if (std::distance(matching_key_tags_anyObject.first, matching_key_tags_anyObject.second) != 0) {
        for (auto tagIt = matching_key_tags_anyObject.first, end = matching_key_tags_anyObject.second; tagIt != end;
             ++tagIt) {
          if (!tagIt->value.empty()) {
            if (thePbfTag.value() == tagIt->value) {
              hitlistAnyType.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
              return;
            }
          } else {
            // this is a tag in the taginfo file that only specifies a key
            hitlistAnyType.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
            return;
          }
        }
    }

    if (std::distance(matching_key_tags.first, matching_key_tags.second) != 0) {
      for (auto tagIt = matching_key_tags.first, end = matching_key_tags.second; tagIt != end; ++tagIt) {
        if (tagIt->value.empty()) {
          // this tag only specified an objectType and a key, it's a hit
          hitlistWithType.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
          return;
        } else {
          if (thePbfTag.value() == tagIt->value) {
            hitlistWithType.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
            return;
          }
        }
      }
    }

    unknown_types.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
  };

  void way(const osmium::Way &way) {
    for (const auto &pbfTag : way.tags()) {
      verifyAndStoreTags(object::type::way, tags_on_ways, pbfTag);
    }
  }

  void node(const osmium::Node &node) {
    for (const auto &pbfTag : node.tags()) {
      verifyAndStoreTags(object::type::node, tags_on_nodes, pbfTag);
    }
  }

  void area(const osmium::Area &area) {
    for (const auto &pbfTag : area.tags()) {
      verifyAndStoreTags(object::type::area, tags_on_areas, pbfTag);
    }
  }

  void relation(const osmium::Relation &rel) {
    for (const auto &pbfTag : rel.tags()) {
      verifyAndStoreTags(object::type::relation, tags_on_relations, pbfTag);
    }
  }

  void printUnknowns() {
    std::cout << "Unrecognized types" << std::endl;
    for (const auto &unknown : unknown_types) {
      std::cout << unknown.type << "\t" << unknown.key << "\t" << unknown.value << std::endl;
    }
  }

  void printMissing() {
    std::cout << "# valid found tags, expected on any object type:" << std::endl;
    for (const auto &Hit : hitlistAnyType) {
        std::cout << Hit << std::endl;
    }

    std::ostream_iterator<tag> out{std::cout, "\n"};

    std::cout << "# Way tags missing in provided pbf" << std::endl;
    std::set_difference(tags_on_ways.first, tags_on_ways.second, hitlistWithType.begin(), hitlistWithType.end(), out, byUniqueHits{});
    std::cout << "# Node tags missing in provided pbf" << std::endl;
    std::set_difference(tags_on_nodes.first, tags_on_nodes.second, hitlistWithType.begin(), hitlistWithType.end(), out,
                        byUniqueHits{});
    std::cout << "# Relation tags missing in provided pbf" << std::endl;
    std::set_difference(tags_on_relations.first, tags_on_relations.second, hitlistWithType.begin(), hitlistWithType.end(), out,
                        byUniqueHits{});
    std::cout << "# Area tags missing in provided pbf" << std::endl;
    std::set_difference(tags_on_areas.first, tags_on_areas.second, hitlistWithType.begin(), hitlistWithType.end(), out,
                        byUniqueHits{});

    std::cout << "# Tags with no object type specified missing in provided pbf" << std::endl;
    std::set_difference(tags_on_any_object.first, tags_on_any_object.second, hitlistAnyType.begin(), hitlistAnyType.end(), out,
                        byUniqueHitsAnyType{});
  }

  struct byUniqueHitsAnyType {
    bool operator()(const tag &lhs, const tag &rhs) {
      if (lhs.value.empty()) {
        return (std::tie(lhs.key) < std::tie(rhs.key));
      } else {
        return (std::tie(lhs.key, lhs.value) < std::tie(rhs.key, rhs.value));
      }
    }
  };

  struct byUniqueHits {
    bool operator()(const tag &lhs, const tag &rhs) {
      return (std::tie(lhs.type, lhs.key, lhs.value) < std::tie(rhs.type, rhs.key, rhs.value));
    }
  };

  // member attributes
  tag_range tags_on_areas;
  tag_range tags_on_nodes;
  tag_range tags_on_ways;
  tag_range tags_on_relations;
  tag_range tags_on_any_object;

  std::unordered_set<tag, boost::hash<tag>> unknown_types;
  std::set<tag> hitlistWithType;
  std::set<tag> hitlistAnyType;
};

#endif
