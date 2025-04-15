#include "grouping.hpp"
#include "partitioning.hpp"

#include <unordered_set>

// https://stackoverflow.com/questions/20590656/error-for-hash-function-of-pair-of-ints
struct pairHashInteger final {
    size_t operator()(const std::pair<int, int>& p) const noexcept {
        size_t hash = std::hash<int>{}(p.first);
        hash <<= sizeof(size_t) * 4;
        hash ^= std::hash<int>{}(p.second);
        return std::hash<size_t>{}(hash);
    }
};

PWayPartitioner::PWayPartitioner(Subgraph *subgraph, int groups) {
    this->subgraph = subgraph;
    this->desiredPartitionCount = groups;

    this->hewgts = nullptr;
    this->eind = nullptr;
    this->eptr = nullptr;
}

PWayPartitioner::~PWayPartitioner(void) {
    this->freeHMETISStructures();
}

void PWayPartitioner::freeHMETISStructures(void) {
    if (this->hewgts != nullptr) {
        delete this->hewgts;
    }

    if (this->eind != nullptr) {
        delete this->eind;
    }

    if (this->eptr != nullptr) {
        delete this->eptr;
    }
}

int PWayPartitioner::doPartition(void) {
    // Map from HMETIS vertex ID to it's connections (hyperedges)
    std::unordered_map<int, std::unordered_set<int>> hyperedges;
    int hyperedgeEndpointCount = 0;

    // First convert the subgraph into the hypergraph format for HMETIS
    this->nvtxs = this->subgraph->getVertices().size();

    // Create HMETIS-specific IDs starting from 0 for each vertex
    // Store a mapping in both directions between the different IDs
    int nextHID = 0;
    this->hmetisIdsMap.clear();
    this->sgraphIdsMap.clear();
    this->sgraphIdsMap.reserve(this->nvtxs);
    for (const auto &[sid, vertex] : this->subgraph->getVertices()) {
        this->hmetisIdsMap[sid] = nextHID;
        this->sgraphIdsMap[nextHID++] = sid;
    }

    // Populate the hyperedge map
    for (const SubgraphEdge *edge : this->subgraph->getEdges()) {
        // Get HMETIS-IDs for origin and target
        int originHID = hmetisIdsMap.at(edge->originNode);
        int targetHID = hmetisIdsMap.at(edge->targetNode);

        // Check if a hyperedge already exists from the origin node
        if (hyperedges.find(originHID) != hyperedges.end()) {
            hyperedges.at(originHID).insert(targetHID);
            // Added one element to existing hyperedge
            hyperedgeEndpointCount += 1;
        } else {
            std::unordered_set<int> newHyperedge;

            newHyperedge.insert(targetHID);
            hyperedges[originHID] = newHyperedge;

            // Added 2 elements: one for the origin and one for the target
            hyperedgeEndpointCount += 2;
        }
    }

    // Set hyperedge count
    this->nhedges = hyperedges.size();

    // Allocate arrays for HMETIS
    this->freeHMETISStructures();
    this->hewgts = new int[this->nhedges];
    this->eptr = new int[this->nhedges + 1];
    this->eind = new int[hyperedgeEndpointCount];

    // Populate HMETIS data structures
    for (const auto &hedgeMembers : hyperedges) {
        // TODO:
    }
}
