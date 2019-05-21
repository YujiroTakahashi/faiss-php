#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/json/php_json.h"
#include "php_faiss.h"
#include "main/SAPI.h"


#include "zend_exceptions.h"
#include "zend_interfaces.h"
#include "SAPI.h"

ZEND_DECLARE_MODULE_GLOBALS(faiss)

/* {{{ PHP_INI
*/
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("faiss.index_dir",  NULL, PHP_INI_SYSTEM, OnUpdateString, index_dir, zend_faiss_globals, faiss_globals)
PHP_INI_END()
/* }}} */

/* Handlers */
static zend_object_handlers faiss_object_handlers;

/* Class entries */
zend_class_entry *php_faiss_sc_entry;

/* {{{ proto void faiss::__construct(int dimension[, string description, int metric])
 */
PHP_METHOD(faiss, __construct)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	zend_long dimension;
	char *description;
	size_t description_len = 0;
	char *default_desc = "Flat";
	zend_long metric = METRIC_L2;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l|sl", 
			&dimension, &description, &description_len, &metric)
	) {
		return;
	}

	if (0 == description_len) {
		description = default_desc;
	}
	faiss_obj->dimension = dimension;

	FAISS_TRY(faiss_index_factory(&faiss_obj->faiss, dimension, description, metric));
}
/* }}} */

/* {{{ proto boolean faiss::isTrained()
 */
PHP_METHOD(faiss, isTrained)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();

	faiss_obj = Z_FAISS_P(object);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (!faiss_Index_is_trained(faiss_obj->faiss)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string faiss::add(int number, array data)
 */
PHP_METHOD(faiss, add)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	zend_long number;
	zval *array;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "lz", &number, &array)) {
		return;
	}

	if (Z_TYPE_P(array) == IS_ARRAY) {
		float *xb;
		int idx, size;
		HashTable *ht = Z_ARRVAL_P(array);
		size = zend_hash_num_elements(ht);

		xb = malloc(size * sizeof(float));
		for (idx=0; idx<size; idx++) {
			zval *value = zend_hash_get_current_data(ht);
			xb[idx] = (float) zval_get_double(value);
			zend_hash_move_forward(ht);
		}

		FAISS_TRY(faiss_Index_add(faiss_obj->faiss, number, xb));
		free(xb);
	}
}
/* }}} */

/* {{{ proto string faiss::addWithIds(int number, array dists, array ids)
 */
PHP_METHOD(faiss, addWithIds)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	zend_long number;
	zval *distVal, *labelVal;
	float *x;
	long *xids;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "lzz", &number, &distVal, &labelVal)) {
		return;
	}
	if (Z_TYPE_P(distVal) != IS_ARRAY || Z_TYPE_P(labelVal) != IS_ARRAY) {
		return;
	}

	{
		int idx, size;
		HashTable *ht = Z_ARRVAL_P(distVal);
		size = zend_hash_num_elements(ht);

		x = malloc(size * sizeof(float));
		for (idx=0; idx<size; idx++) {
			zval *value = zend_hash_get_current_data(ht);
			x[idx] = (float) zval_get_double(value);
			zend_hash_move_forward(ht);
		}
	}

	{
		int idx, size;
		HashTable *ht = Z_ARRVAL_P(labelVal);
		size = zend_hash_num_elements(ht);

		xids = malloc(size * sizeof(long));
		for (idx=0; idx<size; idx++) {
			zval *value = zend_hash_get_current_data(ht);
			xids[idx] = (long) zval_get_long(value);
			zend_hash_move_forward(ht);
		}
	}

	FAISS_TRY(faiss_Index_add_with_ids(faiss_obj->faiss, number, x, xids));
	free(xids);
	free(x);
}
/* }}} */

/* {{{ proto int faiss::ntotal()
 */
PHP_METHOD(faiss, ntotal)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();

	faiss_obj = Z_FAISS_P(object);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}
	ZVAL_LONG(return_value, faiss_Index_ntotal(faiss_obj->faiss));
}
/* }}} */


/* {{{ proto string faiss::search(int number, array query, int k)
 */
PHP_METHOD(faiss, search)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	zend_long number;
	zval *array;
	HashTable *ht;
	zend_long k = 5;
	int idx, size;
	long *labels;
	float *distances, *query;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "lzl", &number, &array, &k)) {
		return;
	}

	if (Z_TYPE_P(array) != IS_ARRAY) {
		return;
	}

	ht = Z_ARRVAL_P(array);
	size = zend_hash_num_elements(ht);

	query = malloc(size * sizeof(float));
	for (idx=0; idx<size; idx++) {
		zval *value = zend_hash_get_current_data(ht);
		distances[idx] = (float) zval_get_double(value);
		zend_hash_move_forward(ht);
	}

	array_init(return_value);
	{
		zval distVal, labelVal;

		array_init(&distVal);
		array_init(&labelVal);

		labels = malloc(k * number * sizeof(long));
		distances = malloc(k * number * sizeof(float));
		FAISS_TRY(faiss_Index_search(faiss_obj->faiss, number, query, k, distances, labels));

		size = k * number;
		for (idx=0; idx<size; idx++) {
			add_index_double(&distVal, idx, distances[idx]);
			add_index_long(&labelVal, idx, labels[idx]);
		}

		zend_hash_str_add(Z_ARRVAL_P(return_value), "distances", sizeof("distances")-1, &distVal);
		zend_hash_str_add(Z_ARRVAL_P(return_value), "labels", sizeof("labels")-1, &labelVal);
	}
}
/* }}} */

/* {{{ proto string faiss::reset()
 */
PHP_METHOD(faiss, reset)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();

	faiss_obj = Z_FAISS_P(object);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	FAISS_TRY(faiss_Index_reset(faiss_obj->faiss));
}
/* }}} */

/* {{{ proto string faiss::reconstruct(long key, array recons)
 */
PHP_METHOD(faiss, reconstruct)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	zend_long key;
	zval *array;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "lz", &key, &array)) {
		return;
	}

	if (Z_TYPE_P(array) == IS_ARRAY) {
		float *recons;
		int idx, size;
		HashTable *ht = Z_ARRVAL_P(array);
		size = zend_hash_num_elements(ht);

		recons = malloc(size * sizeof(float));
		for (idx=0; idx<size; idx++) {
			zval *value = zend_hash_get_current_data(ht);
			recons[idx] = (float) zval_get_double(value);
			zend_hash_move_forward(ht);
		}

		FAISS_TRY(faiss_Index_reconstruct(faiss_obj->faiss, key, recons));
		free(recons);
	}
}
/* }}} */

/* {{{ proto string faiss::writeIndex(string filename)
 */
PHP_METHOD(faiss, writeIndex)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	char *fname;
	size_t fname_len = 0;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &fname, &fname_len)) {
		return;
	}

	FAISS_TRY(faiss_write_index_fname(faiss_obj->faiss, fname));
}
/* }}} */

/* {{{ proto string faiss::readIndex(string filename)
 */
PHP_METHOD(faiss, readIndex)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	char *fname;
	size_t fname_len = 0;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &fname, &fname_len)) {
		return;
	}

	FAISS_TRY(faiss_read_index_fname(faiss_obj->faiss, FAISS_C_IO_FLAG_ONDISK_SAME_DIR, fname));
}
/* }}} */

/* {{{ proto string faiss::importIndex(string data)
 */
PHP_METHOD(faiss, importIndex)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	char *data;
	size_t data_len = 0, size = 0;
	FILE *fp;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &data, &data_len)) {
		return;
	}

	size = ('\0' == data[data_len]) ? data_len - 1 : data_len;
	fp = fmemopen((void*)data, size, "rb");

	FAISS_TRY(faiss_read_index(fp, FAISS_C_IO_FLAG_MMAP, &faiss_obj->faiss));
	fclose(fp);
}
/* }}} */

/* {{{ proto string faiss::exportIndex(string filename)
 */
PHP_METHOD(faiss, exportIndex)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	char *data;
	size_t size = 0;
	FILE *fp;

	faiss_obj = Z_FAISS_P(object);

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	fp = open_memstream(data, &size);

	FAISS_TRY(faiss_write_index(faiss_obj->faiss, fp));

	ZVAL_STRINGL(return_value, data, size);

	fclose(fp);
	free(data);
}
/* }}} */



/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_faiss_void, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_id, 0, 0, 1)
	ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_ctor, 0, 0, 1)
	ZEND_ARG_INFO(0, dimension)
	ZEND_ARG_INFO(0, description)
	ZEND_ARG_INFO(0, metric)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_vector, 0, 0, 2)
	ZEND_ARG_INFO(0, number)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_addwids, 0, 0, 3)
	ZEND_ARG_INFO(0, number)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, ids)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_search, 0, 0, 3)
	ZEND_ARG_INFO(0, number)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, k)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_file, 0, 0, 1)
	ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_data, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

/* }}} */


/* {{{ php_sfaiss_class_methods */
static zend_function_entry php_faiss_class_methods[] = {
	PHP_ME(faiss, __construct,             arginfo_faiss_ctor,     ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(faiss, isTrained,               arginfo_faiss_void,     ZEND_ACC_PUBLIC)
	PHP_ME(faiss, add,                     arginfo_faiss_vector,   ZEND_ACC_PUBLIC)
	PHP_ME(faiss, addWithIds,              arginfo_faiss_addwids,  ZEND_ACC_PUBLIC)
	PHP_ME(faiss, ntotal,                  arginfo_faiss_void,     ZEND_ACC_PUBLIC)
	PHP_ME(faiss, search,                  arginfo_faiss_search,   ZEND_ACC_PUBLIC)
	PHP_ME(faiss, reset,                   arginfo_faiss_void,     ZEND_ACC_PUBLIC)
	PHP_ME(faiss, reconstruct,             arginfo_faiss_vector,   ZEND_ACC_PUBLIC)
	PHP_ME(faiss, writeIndex,              arginfo_faiss_file,     ZEND_ACC_PUBLIC)
	PHP_ME(faiss, readIndex,               arginfo_faiss_file,     ZEND_ACC_PUBLIC)
	PHP_ME(faiss, importIndex,             arginfo_faiss_data,     ZEND_ACC_PUBLIC)
	PHP_ME(faiss, exportIndex,             arginfo_faiss_void,     ZEND_ACC_PUBLIC)

	PHP_FE_END
};
/* }}} */

static void php_faiss_object_free_storage(zend_object *object) /* {{{ */
{
	php_faiss_object *intern = php_faiss_from_obj(object);

	if (!intern) {
		return;
	}

	if (intern->faiss) {
		FaissFree(intern->faiss);
		intern->faiss = NULL;
	}

	zend_object_std_dtor(&intern->zo);
}
/* }}} */

static zend_object *php_faiss_object_new(zend_class_entry *class_type) /* {{{ */
{
	php_faiss_object *intern;

	/* Allocate memory for it */
	int faisssize = FaissSize();
	intern = ecalloc(1, sizeof(php_faiss_object) + zend_object_properties_size(class_type) + faisssize);

	zend_object_std_init(&intern->zo, class_type);
	object_properties_init(&intern->zo, class_type);

	intern->zo.handlers = &faiss_object_handlers;

	return &intern->zo;
}
/* }}} */


/* {{{ PHP_MINIT_FUNCTION
*/
PHP_MINIT_FUNCTION(faiss)
{
	zend_class_entry ce;

	memcpy(&faiss_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	/* Register Faiss Class */
	INIT_CLASS_ENTRY(ce, "Croco\\FAISS\\Index", php_faiss_class_methods);
	ce.create_object = php_faiss_object_new;
	faiss_object_handlers.offset = XtOffsetOf(php_faiss_object, zo);
	faiss_object_handlers.clone_obj = NULL;
	faiss_object_handlers.free_obj = php_faiss_object_free_storage;
	php_faiss_sc_entry = zend_register_internal_class(&ce);

	REGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_MSHUTDOWN_FUNCTION(faiss)
{
	UNREGISTER_INI_ENTRIES();

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(faiss)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "SentencePieceProcessor support", "enabled");
	php_info_print_table_row(2, "SentencePieceProcessor module version", PHP_FAISS_VERSION);
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(faiss)
{
	memset(faiss_globals, 0, sizeof(*faiss_globals));
}
/* }}} */

/* {{{ faiss_module_entry
*/
zend_module_entry faiss_module_entry = {
	STANDARD_MODULE_HEADER,
	"faiss",
	NULL,
	PHP_MINIT(faiss),
	PHP_MSHUTDOWN(faiss),
	NULL,
	NULL,
	PHP_MINFO(faiss),
	PHP_FAISS_VERSION,
	PHP_MODULE_GLOBALS(faiss),
	PHP_GINIT(faiss),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_FAISS
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(faiss)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
