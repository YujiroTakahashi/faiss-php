#include <string>
#include <queue>
#include <unordered_map>
#include <vector>

#include <cstdio>
#include <cstdlib>

#include "faiss_api.h"

/**
 * get Faiss Index size
 *
 * @access public
 * @return int
 */
int FaissIndexSize()
{
    return sizeof(struct faiss::Index);
}

/**
 * get stats format
 *
 * @access public
 * @return Stats*
 */
stats_t *FaissStatsFormat(float *distances, long *labels, size_t *size)
{
    std::unordered_map<long, std::vector<size_t>> sumidx;
    for (size_t idx=0; idx < *size; idx++) {
        if (sumidx.find(labels[idx]) == sumidx.end()) {
            std::vector<size_t> idxs = { idx };
            sumidx.insert(std::make_pair(labels[idx], idxs));
        } else {
            sumidx.at(labels[idx]).push_back(idx);
        }
    }

    std::priority_queue<struct _stats> queue;
    for (auto row : sumidx) {
        struct _stats val;
        val.id = row.first;
        val.count = row.second.size();

        float distance = 0.0f;
        for (auto idx : row.second) {
            distance += distances[idx];
        }
        val.distance = distance / val.count;
        queue.push(val);
    }

    *size = queue.size();
    struct _stats *result = (struct _stats*)malloc(*size * sizeof(struct _stats));
    for (size_t idx=0; idx < *size; idx++) {
        result[idx].id = queue.top().id;
        result[idx].count = queue.top().count;
        result[idx].distance = queue.top().distance;
        queue.pop();
    }

    return result;
}