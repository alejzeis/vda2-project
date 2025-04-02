// Based on fastplace main.cpp
#include <iostream>
#include <cstring>

#include "suraj_parser.h"
#include "placer.hpp"
#include "netlist.hpp"

int main(int argv, char *argc[])
{
    char inareFileName[100];
    char innetFileName[100];
    char inPadLocationFileName[100];

    if (argv!=2) {
        std::cout << "Please provide a circuit file name with no extension." << std::endl;
        return 1;
    }

    std::cout << "Reading circuit file " << argc[1] << std::endl;

    Netlist *netlist = new Netlist();
    netlist->loadFromDisk("c17.isc");

    std::cout << *netlist << std::endl;

    netlist->saveHypergraphFile("c17");

    delete netlist;
#if 0
    strcpy (inareFileName, argc[1]);
    strcat(inareFileName, ".are");
    strcpy(innetFileName,argc[1]);
    strcat(innetFileName,".net");
    strcpy(inPadLocationFileName,argc[1]);
    strcat(inPadLocationFileName,".kiaPad");

    int success = parseIbmFile(inareFileName, innetFileName, inPadLocationFileName);
    if (success == -1) {
        cout << "Error reading input file(s)" << endl;
        return 0;
    }

    printf("\nNumber of vertices,hyper = %d %d\n",numCellsAndPads,numhyper);

    PA3Placement::AnalyticPlacer placer;
    placer.doPlacement();

    free(pinLocations);
    free(hEdge_idxToFirstEntryInPinArray);
    free(cellPinArray);
    free(hyperwts);
#endif
}
