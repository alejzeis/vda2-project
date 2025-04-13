// Super-cell Grouping Module (k = 4 default)
// Includes logic level computation without needing precomputed topological sort
#include "grouping.hpp"

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <map>
#include <queue>
#include <cmath>
#include <algorithm>
#include <fstream>

#include "netlist.hpp"
#include "util.hpp"

using namespace std;

const int GROUP_SIZE_K = 4; // default grouping size

static const int MAX_SEARCH_LEVEL = 2;
static const int NORMALIZATION_FACTOR = MAX_SEARCH_LEVEL;

// Map from level to all gates at that level
map<int, vector<int>> levelToNodes;
unordered_map<int, int> nodeLevelMap;

unordered_map<int, Subgraph*> subgraphs;

/**
 * Makes a subgraph from the main Netlist. Only adds nodes
 * of the specified logic level to the map
 */
static Subgraph* makeSubgraph(Netlist &netlist, int logicLevel) {
    Subgraph *graph = nullptr;

    if (levelToNodes.find(logicLevel) != levelToNodes.end()) {
        graph = new Subgraph();

        // Add verticies from all nodes at this logic level
        for (const int gateID : levelToNodes.at(logicLevel)) {
            SubgraphVertex vert;
            vert.id = gateID;

            graph->addVertex(vert);
        }
    }

    return graph;
}

Subgraph::~Subgraph() {
    for (auto &edge : this->edges) {
        delete edge;
    }
}

void Subgraph::addVertex(const SubgraphVertex &vertex) {
    this->graph[vertex.id] = vertex;
}

SubgraphVertex& Subgraph::getVertex(int id) {
    return this->graph.at(id);
}

void Subgraph::addEdge(int origin, int target, double weight) {
    SubgraphEdge *edge;
    SubgraphVertex &originVert = this->graph.at(origin);
    SubgraphVertex &targetVert = this->graph.at(target);

    // Check if an edge already exists in the graph
    if (originVert.connections.find(target) != originVert.connections.end()) {
        edge = originVert.connections.at(target);

        // Update weight by adding
        edge->weight += weight;
    } else {
        edge = new SubgraphEdge;
        edge->originNode = origin;
        edge->targetNode = target;
        edge->weight = weight;

        // Add edge to graph
        this->edges.insert(edge);
        originVert.connections[target] = edge;
        targetVert.connections[origin] = edge;
    }
}

void Subgraph::dump(std::ostream &out) const {
    for (const auto &[id, vert] : this->graph) {
        std::cout << id << " connections and weights:" << std::endl;
        for (const auto &[target, con] : vert.connections) {
            std::cout << "  -> " << target << " (weight: " << con->weight << ")" << std::endl;
        }
    }
}

std::ostream& operator<<(std::ostream &out, const Subgraph &subgraph) {
    subgraph.dump(out);
    return out;
}

std::unordered_set<NetlistNode*> findNeighbors(Netlist &netlist, int baseNode, int maxSearchLevel) {
    std::unordered_set<NetlistNode*> neighbors;
    std::queue<NetlistNode*> Q;

    Q.push(&netlist.at(baseNode));

    while(!Q.empty()) {
        int levelDiff;
        NetlistNode *front = Q.front();
        Q.pop();

        levelDiff = abs(nodeLevelMap[front->id] - nodeLevelMap[baseNode]);
        if (levelDiff <= maxSearchLevel) {
            // Base node is not a neighbor of itself
            if (front->id != baseNode) {
                neighbors.insert(front);
            }

            // Foreach child node
            for (const int child : front->fanOutList) {
                if (abs(nodeLevelMap.at(child) - nodeLevelMap.at(baseNode)) <= maxSearchLevel
                        && neighbors.find(&netlist.at(child)) == neighbors.end()) {
                    Q.push(&netlist.at(child));
                }
            }
            
            // Foreach parent node
            for (const int parent : front->fanInList) {
                if (abs(nodeLevelMap.at(parent) - nodeLevelMap.at(baseNode)) <= maxSearchLevel
                        && neighbors.find(&netlist.at(parent)) == neighbors.end()) {
                    Q.push(&netlist.at(parent));
                }
            }
        }
    }

    return neighbors;
}

void computeLogicLevels(Netlist &netlist) {
    unordered_map<int, int> inDegree;
    for (const auto &[id, gate] : netlist) {
        inDegree[id] = gate.fanInList.size();
        nodeLevelMap[id] = 0;
    }

    queue<int> q;
    for (const auto &[id, deg] : inDegree)
        if (deg == 0) q.push(id);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        int uLevel = nodeLevelMap[u];
        for (int v : netlist[u].fanOutList) {
            nodeLevelMap[v] = max(nodeLevelMap[v], uLevel + 1);
            if (--inDegree[v] == 0) q.push(v);
        }
    }

    for (const auto &[id, level] : nodeLevelMap) {
        std::cout << "Node: " << id << ", level: " << level << std::endl;
        levelToNodes[level].push_back(id);
    }
}

// Super-cell ID assignment: cellID -> groupID
unordered_map<int, int> superCellMap;

void connectivityGraphProcessing(Netlist &netlist) {
    for (const auto &[level, nodes] : levelToNodes) {
        for (const int u : nodes) {
            for (const int v : nodes) {
                if (u != v) {
                    std::unordered_set<NetlistNode*> uNeighbors;
                    std::unordered_set<NetlistNode*> vNeighbors;
                    std::unordered_set<NetlistNode*> commonNeighbors;

                    NetlistNode *uNode = &netlist.at(u);
                    NetlistNode *vNode = &netlist.at(v);

                    // TODO: cache neighbors for each node ahead of time
                    uNeighbors = findNeighbors(netlist, uNode->id, MAX_SEARCH_LEVEL);
                    vNeighbors = findNeighbors(netlist, vNode->id, MAX_SEARCH_LEVEL);

                    commonNeighbors = uNeighbors;
                    commonNeighbors.merge(vNeighbors);

                    for (const auto &neighbor : commonNeighbors) {
                        if (u != neighbor->id && v != neighbor->id) {
                            int levelDiff = abs(nodeLevelMap[u] - nodeLevelMap[neighbor->id]);
                            double edgeWeight = NORMALIZATION_FACTOR / (levelDiff * 1.0);

                            subgraphs.at(level)->addEdge(u, v, edgeWeight);
#if 0
                            std::cout << "Edge Weight added between " << u << " (" << nodeLevelMap[u] << ")";
                            std::cout << " and " << neighbor->id << " (" << nodeLevelMap[neighbor->id] << ")";
                            std::cout << " is " << edgeWeight;
                            std::cout << " (v is " << v << ")" << std::endl;
#endif
                        }
                    }
                }
            }
        }
    }
}

void groupCells(Netlist &netlist) {
    int superCellID = 0;
    for (const auto &[level, nodes] : levelToNodes) {
        for (size_t i = 0; i < nodes.size(); i += GROUP_SIZE_K) {
            for (size_t j = i; j < min(i + GROUP_SIZE_K, nodes.size()); ++j) {
                superCellMap[nodes[j]] = superCellID;
            }
            superCellID++;
        }
    }
}

void writeGroupingToFile(const string &filename) {
    ofstream out(filename);
    for (const auto &[node, group] : superCellMap) {
        out << "n" << node << " -> SuperCell " << group << "\n";
    }
    out.close();
    cout << "Super-cell mapping written to " << filename << endl;
}

void doGrouping(Netlist &netlist) {
    // Dummy parser: insert gate parsing code here or link to your existing parser
    // Example: parsingCircuitFile("b15_1.isc", netlist);

    computeLogicLevels(netlist);

    // Create the subgraphs for each logic level
    for (const auto &[level, nodes] : levelToNodes) {
        subgraphs[level] = makeSubgraph(netlist, level);
    }

    connectivityGraphProcessing(netlist);

    std::cout << "Subgraph 1" << std::endl << *(subgraphs.at(1)) << std::endl;

    //groupCells(netlist);
    writeGroupingToFile("supercell_mapping.txt");

    // TODO: delete subgraphs?
}
