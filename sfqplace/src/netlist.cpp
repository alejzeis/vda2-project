#include "netlist.hpp"
#include "suraj_parser.h"

#include <iostream>
#include <fstream>
#include <unordered_set>
#include <sstream>
#include <string>

static const int DEFAULT_CELL_AREA = 1;

bool Netlist::loadFromDisk(const std::string &filename) {
    // Modified ChatGPT generated code
    
    bool status = true;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "ISCAS85 Parser: Error opening file: " << filename << std::endl;
        status = false;
    } else {
        std::string line;
        std::unordered_map<std::string, int> nodeNamesToIds;

        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '*') { // Skip comments
                std::istringstream iss(line);
                NetlistNode currentNode;
                int numFanOut, numFanIn;

                // Read in node ID
                iss >> currentNode.id;

                // Check if this node is already in the netlist
                if (this->find(currentNode.id) != this->end()) {
                    // Obtain existing data (fanout list?)
                    currentNode = this->at(currentNode.id);
                }

                iss >> currentNode.name >> currentNode.nodeType;

                // Associate the name to the ID 
                nodeNamesToIds[currentNode.name] = currentNode.id;

                if (ISCAS85_NODE_TYPE_FANOUT_BRANCH == currentNode.nodeType) {
                    // fanout branch type is special
                    std::string fanInNodeName;
                    iss >> fanInNodeName;

                    std::string fault;
                    while (iss >> fault) {
                        // Ignore faults
                    }

                    // Link the fanout node to this fanout branch
                    this->at(nodeNamesToIds.at(fanInNodeName)).fanOutList.insert(currentNode.id);
                    // Add the originating fanout node to our fanin list
                    currentNode.fanInList.insert(nodeNamesToIds.at(fanInNodeName));
                } else {
                    iss >> numFanOut >> numFanIn;

                    if (ISCAS85_NODE_TYPE_INPUT == currentNode.nodeType) {
                        currentNode.isPrimaryInput = true;
                    }
                    
                    std::string fault;
                    while (iss >> fault) {
                        // Ignore faults
                    }
                    
                    if (numFanIn > 0) {
                        std::getline(file, line);

                        std::istringstream fanin_stream(line);
                        int fanin_node;

                        while (fanin_stream >> fanin_node) {
                            currentNode.fanInList.insert(fanin_node);

                            // Look up the fanin node and add us to it's fanout list
                            if (this->find(fanin_node) != this->end()) {
                                // Fanin Node already in the netlist, add us to it
                                this->at(fanin_node).fanOutList.insert(currentNode.id);
                            } else {
                                // Fanin Node not already in the netlist
                                NetlistNode fanInNode;
                                fanInNode.id = fanin_node;
                                fanInNode.fanOutList.insert(currentNode.id);
                            }
                        }
                    }
                }

                (*this)[currentNode.id] = currentNode;
            }
        }

        file.close();
    }

    return status;
}

bool Netlist::saveHypergraphFile(const std::string &outputFilename) {
    // Modified ChatGPT generated code
    
    bool status = true;
    std::ofstream outFile(outputFilename + ".net");
    std::ofstream areaFile(outputFilename + ".are");

    if (!outFile.is_open() || !areaFile.is_open()) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        status = false;
    } else {
        int totalEndpoints = 0;
        int totalCells = this->size();
        int gateCells = 0;
        std::unordered_set<int> uniqueHyperedges;
        
        for (const auto &pair : *this) {
            if (!pair.second.isPrimaryInput) {
                gateCells++;
            }
            totalEndpoints += pair.second.fanInList.size() + 1;
            if (!pair.second.fanInList.empty()) {
                uniqueHyperedges.insert(pair.first);
            }
        }
        
        outFile << totalEndpoints << "\n";
        outFile << uniqueHyperedges.size() << "\n";
        outFile << totalCells << "\n";
        outFile << gateCells << "\n";
        
        for (const auto &pair : *this) {
            const NetlistNode &node = pair.second;
            std::string prefix = (node.isPrimaryInput) ? "p" : "a";

            // Write cell to area file
            // Just give every cell the same area
            areaFile << prefix << node.id << " " << DEFAULT_CELL_AREA << std::endl;
            
            if (node.fanInList.size() > 0) {
                outFile << prefix << node.id << " s 1\n";

                for (int faninNode : node.fanInList) {
                    std::string faninPrefix = (this->at(faninNode).isPrimaryInput) ? "p" : "a";
                    outFile << faninPrefix << faninNode << " l\n";
                }
            }
        }
        outFile.close();
        areaFile.close();
    }

    return status;
}

std::ostream& operator<<(std::ostream &out, const Netlist &netlist) {
    // Modified ChatGPT generated code

    for (const auto &pair : netlist) {
        const NetlistNode &node = pair.second;

        out << "Node " << node.id << " (" << node.name << "): " << node.nodeType
                  << " Fanin: " << node.fanInList.size() << " Fanout: " 
                  << node.fanOutList.size() << "\n";

        if (!node.fanInList.empty()) {
            out << "  Fanin nodes: ";
            for (int fn : node.fanInList) {
                out << fn << " ";
            }
            out << "\n";
        }
    }

    return out;
}

