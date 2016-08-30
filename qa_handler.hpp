#ifndef TAGINFO_VALIDATE_QA_HANDLER
#define TAGINFO_VALIDATE_QA_HANDLER

#include <algorithm>
#include <iterator>
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
  qa_handler(const taginfo_parser &taginfo, std::unordered_map<std::string, uint32_t> &string_catalogue,
             std::unordered_map<uint32_t, std::string> &reverse_string_catalogue, bool unknowns)
      : tags_on_areas(taginfo.tags_on_areas()), tags_on_nodes(taginfo.tags_on_nodes()),
        tags_on_ways(taginfo.tags_on_ways()), tags_on_relations(taginfo.tags_on_relations()),
        tags_on_any_object(taginfo.tags_on_any_object()), ST(string_catalogue), reverse_ST(reverse_string_catalogue),
        store_unknowns(unknowns){};

  using tag_range = std::pair<taginfo_parser::tag_iter, taginfo_parser::tag_iter>;
  struct keyCompare {
    bool operator()(const tag &left, const tag &right) { return left.key < right.key; }
  };

  // Given an object type, a range of tags of that object type and a pbf tag, check
  // if the pbf tag is found in the range of tags, or store the pbf tag to list of unrecognized tags
  void verifyAndStoreTags(const object::type &objectType, const tag_range &tagRange, const osmium::Tag &thePbfTag) {
    // If the key of the pbf tag in question is not already the string catalogue,
    // we can assume that it's not in the provided taginfo file and skip it
    if (ST.find(thePbfTag.key()) == ST.end()) {
      if (store_unknowns) {
        ST.insert({thePbfTag.key(), ST.size()});
        unknown_types.insert(tag{ST[thePbfTag.key()], ST[thePbfTag.value()], objectType});
      }
      return;
    } else {
        ST.insert({thePbfTag.key(), ST.size()});
    }

    // Look for the given pbf tag's key in the range of acceptable keys
    auto range_keymatch_tags =
        std::equal_range(tagRange.first, tagRange.second, tag{ST.at(thePbfTag.key()), 0, objectType}, keyCompare());
    auto range_keymatch_tags_anyObj = std::equal_range(tags_on_any_object.first, tags_on_any_object.second,
                                                        tag{ST.at(thePbfTag.key()), 0, object::type::all}, keyCompare());

    if (std::distance(range_keymatch_tags_anyObj.first, range_keymatch_tags_anyObj.second) == 0 &&
        std::distance(range_keymatch_tags.first, range_keymatch_tags.second) == 0) {
      if (store_unknowns) {
        unknown_types.insert(tag{ST[thePbfTag.key()], ST[thePbfTag.value()], objectType});
      }
      return;
    }

    // first check if given pbf tag matches an entry in the range of taginfo tags
    // with no object type specified because it's more likely to match first
    if (std::distance(range_keymatch_tags_anyObj.first, range_keymatch_tags_anyObj.second) != 0) {
      for (auto tagIt = range_keymatch_tags_anyObj.first, end = range_keymatch_tags_anyObj.second; tagIt != end;
           ++tagIt) {
        if (tagIt->value == 0) {
          // this is a tag in the taginfo file that only specifies a key
          hitlistAnyType.insert(tag{ST.at(thePbfTag.key()), 0, objectType});
          return;
        } else {
          if (thePbfTag.value() == reverse_ST.at(tagIt->value)) {
            ST.insert({thePbfTag.value(), ST.size()});
            hitlistAnyType.insert(tag{ST.at(thePbfTag.key()), ST.at(thePbfTag.value()), objectType});
            return;
          }
        }
      }
    }

    if (std::distance(range_keymatch_tags.first, range_keymatch_tags.second) != 0) {
      for (auto tagIt = range_keymatch_tags.first, end = range_keymatch_tags.second; tagIt != end; ++tagIt) {
        if (tagIt->value == 0) {
          // this tag only specified an objectType and a key, it's a hit
          ST.insert({thePbfTag.key(), ST.size()});
          hitlistWithType.insert(tag{ST.at(thePbfTag.key()), 0, objectType});
          return;
        } else {
          if (thePbfTag.value() == reverse_ST.at(tagIt->value)) {
            hitlistWithType.insert(tag{ST.at(thePbfTag.key()), ST.at(thePbfTag.value()), objectType});
            return;
          }
        }
      }
    }

    if (store_unknowns) {
      ST.insert({thePbfTag.key(), ST.size()});
      ST.insert({thePbfTag.value(), ST.size()});
      unknown_types.insert(tag{ST[thePbfTag.key()], ST[thePbfTag.value()], objectType});
    }
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

  void printTags(std::vector<tag> &tagVector) {
    for (const auto &tag : tagVector) {
      std::cout << tag.type << " " << reverse_ST[tag.key] << "=" << (tag.value == 0 ? "\"\"" : reverse_ST[tag.value])
                << std::endl;
    }
    std::cout << "\n" << std::endl;
  }

  void reverseCatalogue(const std::unordered_map<std::string, uint32_t> &original) {
    std::transform(
        begin(original), end(original), std::inserter(reverse_ST, std::end(reverse_ST)),
        [&](const std::pair<std::string, uint32_t> &entry) { return std::make_pair(entry.second, entry.first); });
  }

  void printMissing() {
    reverseCatalogue(ST);
    std::cout << "# valid found tags, expected on any object type:" << std::endl;
    for (const auto &Hit : hitlistAnyType) {
      std::cout << Hit.type << " " << reverse_ST[Hit.key] << "=" << (Hit.value == 0 ? "\"\"" : reverse_ST[Hit.value]) << std::endl;
    }
    std::cout << "\n" << std::endl;

    std::cout << "# valid found tags, expected on any object type:" << std::endl;
    for (const auto &Hit : hitlistWithType) {
      std::cout << Hit.type << " " << reverse_ST[Hit.key] << "=" << (Hit.value == 0 ? "\"\"" : reverse_ST[Hit.value]) << std::endl;
    }
    std::cout << "\n" << std::endl;

    std::vector<tag> missing_ways;
    std::cout << "# Way tags missing in provided pbf" << std::endl;
    std::set_difference(tags_on_ways.first, tags_on_ways.second, hitlistWithType.begin(), hitlistWithType.end(),
                        std::back_inserter(missing_ways), byUniqueHits{});
    printTags(missing_ways);

    std::vector<tag> missing_nodes;
    std::cout << "# Node tags missing in provided pbf" << std::endl;
    std::set_difference(tags_on_nodes.first, tags_on_nodes.second, hitlistWithType.begin(), hitlistWithType.end(),
                        std::back_inserter(missing_nodes), byUniqueHits{});
    printTags(missing_nodes);

    std::vector<tag> missing_rels;
    std::cout << "# Relation tags missing in provided pbf" << std::endl;
    std::set_difference(tags_on_relations.first, tags_on_relations.second, hitlistWithType.begin(),
                        hitlistWithType.end(), std::back_inserter(missing_rels), byUniqueHits{});
    printTags(missing_rels);

    std::vector<tag> missing_areas;
    std::cout << "# Area tags missing in provided pbf" << std::endl;
    std::set_difference(tags_on_areas.first, tags_on_areas.second, hitlistWithType.begin(), hitlistWithType.end(),
                        std::back_inserter(missing_areas), byUniqueHits{});
    printTags(missing_areas);

    std::vector<tag> missing_any;
    std::cout << "# Tags with no object type specified missing in provided pbf" << std::endl;
    std::set_difference(tags_on_any_object.first, tags_on_any_object.second, hitlistAnyType.begin(),
                        hitlistAnyType.end(), std::back_inserter(missing_any), byUniqueHitsAnyType{});
    printTags(missing_any);
  }

  struct byUniqueHitsAnyType {
    bool operator()(const tag &lhs, const tag &rhs) {
      if (lhs.value == 0) {
        return (std::tie(lhs.key) < std::tie(rhs.key));
      } else {
        return (std::tie(lhs.key, lhs.value) < std::tie(rhs.key, rhs.value));
      }
    }
  };

  struct byUniqueHits {
    bool operator()(const tag &lhs, const tag &rhs) {
      if (lhs.value == 0) {
        return (std::tie(lhs.type, lhs.key) < std::tie(rhs.type, rhs.key));
      } else {
        return (std::tie(lhs.type, lhs.key, lhs.value) < std::tie(rhs.type, rhs.key, rhs.value));
      }
    }
  };

  // member attributes
  tag_range tags_on_areas;
  tag_range tags_on_nodes;
  tag_range tags_on_ways;
  tag_range tags_on_relations;
  tag_range tags_on_any_object;
  // catalogues mapping tag values to integers
  std::unordered_map<std::string, uint32_t> ST;
  std::unordered_map<uint32_t, std::string> reverse_ST;
  bool store_unknowns;

  std::unordered_set<tag, boost::hash<tag>> unknown_types;
  std::set<tag> hitlistWithType;
  std::set<tag> hitlistAnyType;
};

#endif
