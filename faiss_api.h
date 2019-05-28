#ifndef __CROCO_FAISS_API_H
#define __CROCO_FAISS_API_H

#include <stdint.h>

#ifdef __cplusplus

#include <faiss/Index.h>

extern "C" {

#endif /* __cplusplus */

#ifndef EXPLOSION_API
#   if defined(_WIN32) || defined(_WIN64)
#       define EXPLOSION_API __declspec(dllimport)
#   else
#       define EXPLOSION_API extern
#   endif /* defined(_WIN32) || defined(_WIN64) */
#endif /* EXPLOSION_API */

#ifndef EXPLOSION_VERSION
#    define EXPLOSION_VERSION 1
#endif /* EXPLOSION_VERSION */

typedef struct _stats {
    int id;
    int count;
    float distance;
#ifdef __cplusplus
    bool operator<(const struct _stats &stats) const  {
        return distance > stats.distance;
    }
#endif /* __cplusplus */
} stats_t;

EXPLOSION_API int FaissIndexSize();
EXPLOSION_API stats_t *FaissStatsFormat(float *distances, long *labels, size_t *size);

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /* __CROCO_FAISS_API_H */