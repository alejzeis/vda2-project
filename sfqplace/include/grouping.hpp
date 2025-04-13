#ifndef SFQPLACE_GROUPING_HPP
#define SFQPLACE_GROUPING_HPP

#include "netlist.hpp"
#include <ostream>

struct SubgraphEdge {
    int originNode;
    int targetNode;
    double weight;
};

struct SubgraphVertex {
    int id;
    std::unordered_map<int, SubgraphEdge*> connections;
};

class Subgraph {
    public:
        ~Subgraph();

        void addVertex(const SubgraphVertex &vert);
        void addEdge(int origin, int target, double weight);

        SubgraphVertex& getVertex(int id);

        void dump(std::ostream &out) const;
    private:
        std::unordered_map<int, SubgraphVertex> graph;
        std::unordered_set<SubgraphEdge*> edges;
};

std::ostream& operator<<(std::ostream &out, const Subgraph &subgraph);

void doGrouping(Netlist &netlist);

#endif //SFQPLACE_GROUPING_HPP
