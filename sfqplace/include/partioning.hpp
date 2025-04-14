#ifndef SFQPLACE_PARTITIONING_HPP
#define SFQPLACE_PARTITIONING_HPP

#include "grouping.hpp"

// HMETIS C symbols
extern "C" {
    void HMETIS_PartRecursive(int nvtxs, int nhedges, int *vwgts, int *eptr, int *eind, int *hewgts, int nparts, int ubfactor, int *options, int *part, int *edgecut);

    void HMETIS_PartKway(int nvtxs, int nhedges, int *vwgts, int *eptr, int *eind, int *hewgts, int nparts, int ubfactor, int *options, int *part, int *edgecut);
}

class PWayPartioner {
public:
    /**
     * Creates a new P-way partitioner for the given subgraph,
     * preparing to partition it into P groups
     */
    PWayPartioner(Subgraph *subgraph, int groups);

    ~PWayPartioner();

    /**
     * Converts the subgraph internally to an input format readable by HMETIS,
     * then invokes HMETIS to perform the partitioning.
     * Will save the results internally. To obtain which vertexes belong to which partition,
     * call getPartitions() after this.
     *
     * Returns the total number of partitions created.
     */
    int doPartition(void);

    /**
     * Returns a mapping from each vertex ID to the partition it belongs to.
     * doPartition() must have been called before this.
     */
    std::unordered_map<int, int> getPartitions();

private:
    int desiredPartitionCount;

    int nvtxs;
    int nhedges;
    int *eptr;
    int *eind;
};

#endif // SFQPLACE_PARTITIONING_HPP
