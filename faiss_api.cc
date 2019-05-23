#include <string>

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