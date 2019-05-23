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

EXPLOSION_API int FaissIndexSize();

#ifdef __cplusplus
}

#endif /* __cplusplus */

#endif /* __CROCO_FAISS_API_H */