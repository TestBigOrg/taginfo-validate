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
    // Look for the pbf tag's key in the range of expected keys
    auto matching_key_tags =
        std::equal_range(tagRange.first, tagRange.second, tag{thePbfTag.key(), {}, objectType}, keyCompare());
    auto matching_key_tags_anyObject = std::equal_range(tags_on_any_object.first, tags_on_any_object.second,
                                                        tag{thePbfTag.key(), {}, object::type::all}, keyCompare());

    if (std::distance(matching_key_tags_anyObject.first, matching_key_tags_anyObject.second) == 0 &&
        std::distance(matching_key_tags.first, matching_key_tags.second) == 0) {
      unknown_types.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
      return;
    }

    for (auto tagIt = matching_key_tags_anyObject.first, end = matching_key_tags_anyObject.second; tagIt != end;
         ++tagIt) {
      if (!tagIt->value.empty()) {
        if (thePbfTag.value() == tagIt->value) {
          hitlist.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
          return;
        }
      } else {
        // this is a tag in the taginfo file that only specifies a key
        hitlist.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
        return;
      }
    }

    if (std::distance(matching_key_tags.first, matching_key_tags.second) != 0) {
      for (auto tagIt = matching_key_tags.first, end = matching_key_tags.second; tagIt != end; ++tagIt) {
        if (tagIt->value.empty()) {
          // this tag only specified an objectType and a tag key
          hitlist.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
        } else {
          if (thePbfTag.value() != tagIt->value) {
            unknown_types.insert(tag{thePbfTag.key(), thePbfTag.value(), objectType});
            return;
          }
        }
      }
    }
  };

  void way(const osmium::Way &way) {
    for (const auto &pbfTag : way.tags()) {
      verifyAndStoreTags(object::type::way, tags_on_ways, pbfTag);
    }
  }

  /*
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
  */

  void printUnknowns() {
    std::cout << "Unrecognized types" << std::endl;
    for (const auto &unknown : unknown_types) {
      std::cout << unknown.type << "\t" << unknown.key << "\t" << unknown.value << std::endl;
    }
  }

  // compare if lhs is found in rhs
  struct byConditional {
    bool operator()(const tag &lhs, const tag &rhs) const {
      if (lhs.key != rhs.key)
        return false;
      if (!lhs.value.empty() && (lhs.type != object::type::all))
        return (std::tie(lhs.value, lhs.type) < std::tie(rhs.value, rhs.type));
      if (!lhs.value.empty())
        return (lhs.value < rhs.value);
      if (lhs.type)
        return (lhs.type < rhs.type);
      return true;
    }
  };

  void printMissing() {
    std::cout << "Tags found in taginfo.json but not in provided pbf" << std::endl;
    std::ostream_iterator<tag> out{std::cout, "\n"};
    std::set_difference(tags_on_nodes.first, tags_on_nodes.second, hitlist.begin(), hitlist.end(), out,
                        byConditional{});
    std::set_difference(tags_on_ways.first, tags_on_ways.second, hitlist.begin(), hitlist.end(), out, byConditional{});
    std::set_difference(tags_on_relations.first, tags_on_relations.second, hitlist.begin(), hitlist.end(), out,
                        byConditional{});
    std::set_difference(tags_on_areas.first, tags_on_areas.second, hitlist.begin(), hitlist.end(), out,
                        byConditional{});
    std::set_difference(tags_on_any_object.first, tags_on_any_object.second, hitlist.begin(), hitlist.end(), out,
                        byConditional{});
  }

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
  std::set<tag, byUniqueHits> hitlist;
};

#endif
