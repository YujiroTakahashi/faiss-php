#ifndef PHP_FAISS_H
#define PHP_FAISS_H

#include <faiss/c_api/Index.h>

#define PHP_FAISS_VERSION	"0.1.0"

extern zend_module_entry faiss_module_entry;
#define phpext_faiss_ptr &faiss_module_entry

ZEND_BEGIN_MODULE_GLOBALS(faiss)
	char *index_dir;
ZEND_END_MODULE_GLOBALS(faiss)

#ifdef ZTS
# define FAISS_G(v) TSRMG(faiss_globals_id, zend_faiss_globals *, v)
# ifdef COMPILE_DL_FAISS
ZEND_TSRMLS_CACHE_EXTERN()
# endif
#else
# define FAISS_G(v) (faiss_globals.v)
#endif

typedef struct {
    zend_object zo;
	zval error;
    FaissIndex *faiss;
	zend_long dimension;
} php_faiss_object;

static inline php_faiss_object *php_faiss_from_obj(zend_object *obj) {
	return (php_faiss_object*)((char*)(obj) - XtOffsetOf(php_faiss_object, zo));
}

#define Z_FAISS_P(zv) php_faiss_from_obj(Z_OBJ_P((zv)))


#endif  /* PHP_FAISS_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
