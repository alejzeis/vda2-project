#ifndef SFQPLACE_NETLIST_HPP
#define SFQPLACE_NETLIST_HPP

#include <string>
#include <vector>
#include <set>
#include <unordered_map>

static const std::string ISCAS85_NODE_TYPE_INPUT = "inpt";
static const std::string ISCAS85_NODE_TYPE_FANOUT_BRANCH = "from";
static const std::string NODE_TYPE_OUTPUT = "otpt";

struct NetlistNode {
    int id;
    int hyperId;
    std::string name;
    std::string nodeType;
    std::set<int> fanInList;
    std::set<int> fanOutList;
    bool isPrimaryOutput = false;
    bool isPrimaryInput = false;
};

class Netlist : public std::unordered_map<int, NetlistNode> {
public:
    /**
     * Reads a given netlist in the ISCAS '85 format.
     * If the file cannot be read this function will return false.
     * Otherwise, a successful load will return true.
     */
    bool loadFromDisk(const std::string &filename);

    /**
     * Saves the netlist in the hypergraph format used by the FastPlace
     * implementation (.net files). If successful this function returns true,
     * otherwise a return value of false indicates an error occurred.
     */
    bool saveHypergraphFile(const std::string &outFile);
private:
    int nextId;

    /**
     * Iterates through the netlist removing "fanout branches",
     * a special type of node defined by the ISCAS85 format, but useless.
     */
    void eliminateFanoutBranches(void);

    /**
     * Assigns a "hyperId" to each node that does not skip numbers,
     * used to create the hypergraph netlist format.
     */
    void consolidateIds(void);

    /**
     * Adds output pad nodes to any cell with zero fan out.
     */
    void addOutputs(void);
};

std::ostream& operator<<(std::ostream &out, const Netlist &netlist);

#endif // SFQPLACE_NETLIST_HPP
