#ifndef SFQPLACE_NETLIST_HPP
#define SFQPLACE_NETLIST_HPP

#include <string>
#include <vector>
#include <unordered_map>

struct NetlistNode {
    std::string nodeType;
    std::vector<int> fanInList;
    std::vector<int> fanOutList;
    bool isPrimaryOutput = false;
    bool isPrimaryInput = false;
};

typedef std::unordered_map<int, NetlistNode> NetlistMap;

/**
 * Allocates a NetlistMap, loading its contents from the suraj_parser code,
 * which in turn reads data from the ibm files. It is expected that suraj_parser
 * has been invoked already.
 *
 * The caller is responsible for freeing the map once it is no longer needed
 */
NetlistMap* surajParserLoadNetlist(void);

#endif // SFQPLACE_NETLIST_HPP
