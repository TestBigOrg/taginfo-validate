#ifndef TAGINFO_VALIDATE_TAG
#define TAGINFO_VALIDATE_TAG

#include <string>
#include <ostream>

#include <boost/functional/hash.hpp>
#include <boost/filesystem/path.hpp>

namespace taginfo_validate {
  namespace object {
    enum type {
      unknown = 0u,
      //
      node = 1u << 0,
      way = 1u << 1,
      relation = 1u << 2,
      area = 1u << 3,
      //
      all = unknown | node | way | relation | area
    };

    // print type string, not enum
    std::ostream &operator<<(std::ostream &os, const type &rhs) {
        switch (rhs) {
            case type::node:
                os << "node";
                break;
            case type::way:
                os << "way";
                break;
            case type::relation:
                os << "relation";
                break;
            case type::area:
                os << "area";
                break;
            case type::unknown:
                os << "unknown";
                break;
            case type::all:
                os << "all";
                break;
        }
        return os;
    }
  }

  struct tag {
    std::string key;
    std::string value;
    object::type type;
    // make hashable, so tags can be stored in unordered_map
    friend std::size_t hash_value(tag const& t)
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, t.key);
        boost::hash_combine(seed, t.value);
        boost::hash_combine(seed, t.type);

        return seed;
    }
    friend bool operator==(const tag &lhs, const tag &rhs) { return (lhs.key == rhs.key && lhs.value == rhs.value && lhs.type == rhs.type); };
    // overload less than to make sortable
    friend bool operator<(const tag &lhs, const tag &rhs) { return lhs.type < rhs.type; };
    friend std::ostream &operator<<(std::ostream &out, const tag &rhs) { return out << rhs.key << "=" << rhs.value; }
  };
}

#endif
