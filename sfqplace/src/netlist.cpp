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

        this->nextId = 0;

        while (std::getline(file, line)) {
            if (!line.empty() && line[0] != '*') { // Skip comments
                std::istringstream iss(line);
                NetlistNode currentNode;
                int numFanOut, numFanIn;

                // Read in node ID
                iss >> currentNode.id;

                if (currentNode.id > this->nextId) {
                    this->nextId = currentNode.id + 1;
                }

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

        // Add output "pads" for any cell that has a zero fanout
        this->addOutputs();

        file.close();
    }

    return status;
}

bool Netlist::saveHypergraphFile(const std::string &outputFilename) {
    // Modified ChatGPT generated code
    
    bool status = true;
    std::ofstream outFile(outputFilename + ".net");
    std::ofstream areaFile(outputFilename + ".are");

    // Assign "hyperId" to each node
    // Removes gaps in the id space, parser may not work without it
    this->consolidateIds();

    if (!outFile.is_open() || !areaFile.is_open()) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        status = false;
    } else {
        int totalEndpoints = 0;
        int totalCells = this->size();
        int gateCells = 0;
        std::unordered_set<int> uniqueHyperedges;
        
        for (const auto &pair : *this) {
            std::cout << "Node " << pair.first << "type: " << pair.second.nodeType << std::endl;
            if (!pair.second.isPrimaryInput) {
                gateCells++;
            }
            if (!pair.second.fanOutList.empty()) {
                totalEndpoints += pair.second.fanOutList.size() + 1;
                uniqueHyperedges.insert(pair.first);
            }
        }
        
        outFile << 0 << std::endl;
        outFile << totalEndpoints << "\n";
        outFile << uniqueHyperedges.size() << "\n";
        outFile << totalCells << "\n";
        outFile << gateCells - 1 << "\n"; // suraj_Parser adds 1 for some reason
        
        for (const auto &pair : *this) {
            const NetlistNode &node = pair.second;
            bool cellIsPad = node.isPrimaryOutput || node.isPrimaryInput;
            if (!cellIsPad) {
                std::string prefix = (cellIsPad) ? "p" : "a";

                // Write cell to area file
                // Just give every cell the same area
                areaFile << prefix << node.hyperId << " " << (cellIsPad ? 0 : DEFAULT_CELL_AREA) << std::endl;
                
                if (node.fanOutList.size() > 0) {
                    outFile << prefix << node.hyperId << " s 1\n";

                    for (int fanoutNode : node.fanOutList) {
                        std::string fanoutPrefix = (this->at(fanoutNode).isPrimaryInput) ? "p" : "a";
                        outFile << fanoutPrefix << this->at(fanoutNode).hyperId << " l\n";
                    }
                }
            }
        }
        for (const auto &pair : *this) {
            const NetlistNode &node = pair.second;
            bool cellIsPad = node.isPrimaryOutput || node.isPrimaryInput;
            if (cellIsPad) {
                std::string prefix = (cellIsPad) ? "p" : "a";

                // Write cell to area file
                // Just give every cell the same area
                areaFile << prefix << node.hyperId << " " << (cellIsPad ? 0 : DEFAULT_CELL_AREA) << std::endl;
                
                if (node.fanOutList.size() > 0) {
                    outFile << prefix << node.hyperId << " s 1\n";

                    for (int fanoutNode : node.fanOutList) {
                        std::string fanoutPrefix = (this->at(fanoutNode).isPrimaryInput) ? "p" : "a";
                        outFile << fanoutPrefix << this->at(fanoutNode).hyperId << " l\n";
                    }
                }
            }
        }
        outFile.close();
        areaFile.close();
    }

    return status;
}

void Netlist::eliminateFanoutBranches(void) {
    unordered_set<int> fanoutBranches;

    for (const auto &pair : *this) {
        const NetlistNode &node = pair.second;

        if (ISCAS85_NODE_TYPE_FANOUT_BRANCH == node.nodeType) {
            // Eliminate this node 
            fanoutBranches.insert(pair.first);

            // Fanout branches have a fanin and fanout of 1, so set is only 1 element
            int fanInNode = *(node.fanInList.begin());
            int fanOutNode = *(node.fanOutList.begin());

            // Manipulate fanin, fanout lists of these two nodes to link them together

            // Disconnect us from fanInNode's fanOut list
            this->at(fanInNode).fanOutList.erase(node.id);
            this->at(fanInNode).fanOutList.insert(fanOutNode);

            // Disconnect us from fanOutNode's fanIn list
            this->at(fanOutNode).fanInList.erase(node.id);
            this->at(fanOutNode).fanInList.insert(fanInNode);
        }
    }

    for (const auto &it : fanoutBranches) {
        this->erase(it);
    }
}

void Netlist::addOutputs(void) {
    stringstream outputNodeName;

    // First eliminate fanout branches
    this->eliminateFanoutBranches();

    // Iterate and find nodes with zero fanOut
    for (auto &pair : *this) {
        if (pair.second.fanOutList.empty() && pair.second.nodeType != NODE_TYPE_OUTPUT) {
            NetlistNode outputNode;

            outputNodeName.clear();
            outputNodeName << outputNode.id << "OUT";

            // Add a new output node (represents an I/O pad)
            // and connect the cell to it
            outputNode.id = this->nextId++;
            outputNode.isPrimaryOutput = true;
            outputNode.fanInList.insert(pair.first);
            outputNode.name = outputNodeName.str();
            outputNode.nodeType = NODE_TYPE_OUTPUT;

            pair.second.fanOutList.insert(outputNode.id);

            (*this)[outputNode.id] = outputNode;
        }
    }
}

void Netlist::consolidateIds(void) {
    int moveableCellCounter = 0;
    int padCounter = 1;

    for (auto &pair : *this) {
        if (pair.second.isPrimaryInput || pair.second.isPrimaryOutput) {
            pair.second.hyperId = padCounter++;
        } else {
            pair.second.hyperId = moveableCellCounter++;
        }
    }
}


std::ostream& operator<<(std::ostream &out, const Netlist &netlist) {
    // Modified ChatGPT generated code

    for (const auto &pair : netlist) {
        const NetlistNode &node = pair.second;

        out << "Node " << node.id << " (" << node.name << ", hyper: " << node.hyperId << "): " << node.nodeType
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

