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

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "l|sl", 
			&dimension, &description, &description_len, &metric)
	) {
		return;
	}

	faiss_obj = Z_FAISS_P(object);

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

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	faiss_obj = Z_FAISS_P(object);

	if (!faiss_Index_is_trained(faiss_obj->faiss)) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto string faiss::add(array data, int number)
 */
PHP_METHOD(faiss, add)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	zend_long number = 0;
	zval *array;
	float *xb;

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z|l", &array, &number)) {
		return;
	}

	if (Z_TYPE_P(array) != IS_ARRAY) {
		return ;
	}

	faiss_obj = Z_FAISS_P(object);

	if (0 == number) {
		HashTable *oht = Z_ARRVAL_P(array);
		size_t idx = 0, oidx;
		number = zend_hash_num_elements(oht);

		xb = malloc(faiss_obj->dimension * number * sizeof(float));

		for (oidx=0; oidx<number; oidx++) {
			zval *vecs = zend_hash_get_current_data(oht);
			HashTable *vht = Z_ARRVAL_P(vecs);
			size_t vidx;

			for (vidx=0; vidx<faiss_obj->dimension; vidx++) {
				zval *value = zend_hash_get_current_data(vht);
				xb[idx] = (float) zval_get_double(value);
				idx++;
				zend_hash_move_forward(vht);
			}
			zend_hash_move_forward(oht);
		}
	} else {
		size_t idx, size;
		HashTable *ht = Z_ARRVAL_P(array);
		size = zend_hash_num_elements(ht);

		xb = malloc(size * sizeof(float));
		for (idx=0; idx<size; idx++) {
			zval *value = zend_hash_get_current_data(ht);
			xb[idx] = (float) zval_get_double(value);
			zend_hash_move_forward(ht);
		}
	}

	FAISS_TRY(faiss_Index_add(faiss_obj->faiss, number, xb));
	free(xb);
}
/* }}} */

/* {{{ proto string faiss::addWithIds(array dists, array ids, int number)
 */
PHP_METHOD(faiss, addWithIds)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	zend_long number = 0;
	zval *distVal, *labelVal;
	float *x;
	long *xids;

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "zz|l", &distVal, &labelVal, &number)) {
		return;
	}
	if (Z_TYPE_P(distVal) != IS_ARRAY) {
		return;
	}

	faiss_obj = Z_FAISS_P(object);

	if (0 == number) {
		HashTable *oht = Z_ARRVAL_P(distVal);
		size_t idx = 0, oidx;
		number = zend_hash_num_elements(oht);

		x = malloc(faiss_obj->dimension * number * sizeof(float));

		for (oidx=0; oidx<number; oidx++) {
			zval *vecs = zend_hash_get_current_data(oht);
			HashTable *vht = Z_ARRVAL_P(vecs);
			size_t vidx;

			for (vidx=0; vidx<faiss_obj->dimension; vidx++) {
				zval *value = zend_hash_get_current_data(vht);
				x[idx] = (float) zval_get_double(value);
				idx++;
				zend_hash_move_forward(vht);
			}
			zend_hash_move_forward(oht);
		}
	} else {
		size_t idx, size;
		HashTable *ht = Z_ARRVAL_P(distVal);
		size = zend_hash_num_elements(ht);

		x = malloc(size * sizeof(float));
		for (idx=0; idx<size; idx++) {
			zval *value = zend_hash_get_current_data(ht);
			x[idx] = (float) zval_get_double(value);
			zend_hash_move_forward(ht);
		}
	}

	if (Z_TYPE_P(labelVal) == IS_ARRAY) {
		size_t idx, size;
		HashTable *ht = Z_ARRVAL_P(labelVal);
		size = zend_hash_num_elements(ht);

		xids = malloc(size * sizeof(long));
		for (idx=0; idx<size; idx++) {
			zval *value = zend_hash_get_current_data(ht);
			xids[idx] = (long) zval_get_long(value);
			zend_hash_move_forward(ht);
		}
	} else {
		size_t idx;
		xids = malloc(number * sizeof(long));

		for (idx=0; idx<number; idx++) {
			xids[idx] = (long) zval_get_long(labelVal);
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

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	faiss_obj = Z_FAISS_P(object);
	ZVAL_LONG(return_value, faiss_Index_ntotal(faiss_obj->faiss));
}
/* }}} */


/* {{{ proto array faiss::search(array query[, int k, int format, int number])
 */
PHP_METHOD(faiss, search)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	zend_long number = 0;
	zval *array;
	zend_long k = 5;
	zend_long format = FAISS_C_FORMAT_PLAIN;
	long *labels;
	float *distances, *query;

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "z|lll", &array, &k, &format, &number)) {
		return;
	}

	if (Z_TYPE_P(array) != IS_ARRAY) {
		return;
	}

	faiss_obj = Z_FAISS_P(object);

	if (0 == number) {
		HashTable *oht = Z_ARRVAL_P(array);
		size_t idx = 0, oidx;
		number = zend_hash_num_elements(oht);

		query = malloc(faiss_obj->dimension * number * sizeof(float));

		for (oidx=0; oidx<number; oidx++) {
			zval *vecs = zend_hash_get_current_data(oht);
			HashTable *vht = Z_ARRVAL_P(vecs);
			size_t vidx;

			for (vidx=0; vidx<faiss_obj->dimension; vidx++) {
				zval *value = zend_hash_get_current_data(vht);
				query[idx] = (float) zval_get_double(value);
				idx++;
				zend_hash_move_forward(vht);
			}
			zend_hash_move_forward(oht);
		}
	} else {
		HashTable *ht = Z_ARRVAL_P(array);
		size_t idx, size = zend_hash_num_elements(ht);

		query = malloc(size * sizeof(float));
		for (idx=0; idx<size; idx++) {
			zval *value = zend_hash_get_current_data(ht);
			query[idx] = (float) zval_get_double(value);
			zend_hash_move_forward(ht);
		}
	}

	array_init(return_value);
	labels = malloc(k * number * sizeof(long));
	distances = malloc(k * number * sizeof(float));
	FAISS_TRY(faiss_Index_search(faiss_obj->faiss, number, query, k, distances, labels));

	if (FAISS_C_FORMAT_PLAIN == format) {
		size_t idx, size = k * number;
		for (idx=0; idx<size; idx++) {
			zval rowVal, rankVal, distVal, labelVal;

			ZVAL_LONG(&rankVal, idx + 1);
			ZVAL_DOUBLE(&distVal, distances[idx]);
			ZVAL_LONG(&labelVal, labels[idx]);

			array_init(&rowVal);
			zend_hash_str_add(Z_ARRVAL_P(&rowVal), "Rank", sizeof("Rank")-1, &rankVal);
			zend_hash_str_add(Z_ARRVAL_P(&rowVal), "ID", sizeof("ID")-1, &labelVal);
			zend_hash_str_add(Z_ARRVAL_P(&rowVal), "Distance", sizeof("Distance")-1, &distVal);

			add_index_zval(return_value, idx, &rowVal);
		}
	} else {
		size_t idx, size = k * number;
		stats_t *stats = FaissStatsFormat(distances, labels, &size);
		for (idx=0; idx<size; idx++) {
			zval rowVal, rankVal, countVal, distVal, labelVal;

			ZVAL_LONG(&rankVal, idx + 1);
			ZVAL_LONG(&labelVal, stats[idx].id);
			ZVAL_LONG(&countVal, stats[idx].count);
			ZVAL_DOUBLE(&distVal, stats[idx].distance);

			array_init(&rowVal);
			zend_hash_str_add(Z_ARRVAL_P(&rowVal), "Rank", sizeof("Rank")-1, &rankVal);
			zend_hash_str_add(Z_ARRVAL_P(&rowVal), "ID", sizeof("ID")-1, &labelVal);
			zend_hash_str_add(Z_ARRVAL_P(&rowVal), "Count", sizeof("Count")-1, &countVal);
			zend_hash_str_add(Z_ARRVAL_P(&rowVal), "Distance", sizeof("Distance")-1, &distVal);

			add_index_zval(return_value, idx, &rowVal);
		}
		free(stats);
	}

	free(distances);
	free(labels);
	free(query);
}
/* }}} */

/* {{{ proto void faiss::reset()
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

/* {{{ proto void faiss::reconstruct(long key, array recons)
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

	FAISS_TRY(faiss_read_index_fname(fname, FAISS_C_IO_FLAG_ONDISK_SAME_DIR, &faiss_obj->faiss));
}
/* }}} */

/* {{{ proto string faiss::importIndex(string data)
 */
PHP_METHOD(faiss, importIndex)
{
	php_faiss_object *faiss_obj;
	zval *object = getThis();
	char *data;
	size_t data_len = 0;
	FILE *fp;

	faiss_obj = Z_FAISS_P(object);

	if (FAILURE == zend_parse_parameters_throw(ZEND_NUM_ARGS(), "s", &data, &data_len)) {
		return;
	}
	fp = fmemopen((void*)data, data_len, "rb");

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

	fp = open_memstream(&data, &size);

	FAISS_TRY(faiss_write_index(faiss_obj->faiss, fp));

	fflush(fp);
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

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_vector, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_addwids, 0, 0, 2)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_INFO(0, ids)
	ZEND_ARG_INFO(0, number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_faiss_search, 0, 0, 1)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, k)
	ZEND_ARG_INFO(0, format)
	ZEND_ARG_INFO(0, number)
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
		faiss_Index_free(intern->faiss);
		intern->faiss = NULL;
	}

	zend_object_std_dtor(&intern->zo);
}
/* }}} */

static zend_object *php_faiss_object_new(zend_class_entry *class_type) /* {{{ */
{
	php_faiss_object *intern;

	/* Allocate memory for it */
	int faisssize = FaissIndexSize();
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
	INIT_CLASS_ENTRY(ce, "Croco\\faiss", php_faiss_class_methods);
	ce.create_object = php_faiss_object_new;
	faiss_object_handlers.offset = XtOffsetOf(php_faiss_object, zo);
	faiss_object_handlers.clone_obj = NULL;
	faiss_object_handlers.free_obj = php_faiss_object_free_storage;
	php_faiss_sc_entry = zend_register_internal_class(&ce);

	REGISTER_LONG_CONSTANT("Croco\\faiss\\METRIC_INNER_PRODUCT", 0, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("Croco\\faiss\\METRIC_L2",            1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("Croco\\faiss\\FORMAT_PLAIN",         1, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("Croco\\faiss\\FORMAT_STATS",         2, CONST_CS | CONST_PERSISTENT);

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
