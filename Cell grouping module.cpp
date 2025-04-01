// Super-cell Grouping Module (k = 4 default)
// Includes logic level computation without needing precomputed topological sort

#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>
#include <queue>
#include <set>
#include <cmath>
#include <algorithm>
#include <fstream>

using namespace std;

struct cktGates {
    string nodeType;
    vector<int> fanInList;
    vector<int> fanOutList;
    bool isPrimaryOutput = false;
    bool isPrimaryInput = false;
};

unordered_map<int, cktGates> netlist; // assume filled from parsing the circuit

const int GROUP_SIZE_K = 4; // default grouping size

// Map from level to all gates at that level
map<int, vector<int>> levelToNodes;
unordered_map<int, int> nodeLevelMap;

void computeLogicLevels() {
    unordered_map<int, int> inDegree;
    for (const auto &[id, gate] : netlist)
        inDegree[id] = gate.fanInList.size();

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

    for (const auto &[id, level] : nodeLevelMap)
        levelToNodes[level].push_back(id);
}

// Super-cell ID assignment: cellID -> groupID
unordered_map<int, int> superCellMap;

void groupCells() {
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

int main() {
    // Dummy parser: insert gate parsing code here or link to your existing parser
    // Example: parsingCircuitFile("b15_1.isc", netlist);

    computeLogicLevels();
    groupCells();
    writeGroupingToFile("supercell_mapping.txt");
    return 0;
}
