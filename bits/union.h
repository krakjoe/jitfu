/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2013 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: krakjoe@php.net                                              |
  +----------------------------------------------------------------------+
*/
#ifndef HAVE_BITS_UNION_H
#define HAVE_BITS_UNION_H

zend_class_entry *jit_union_ce;

typedef union _php_jit_union_t {
	zend_object std;
	zend_object_handle h;
	jit_type_t       type;
	php_jit_type_t **fields;
	zend_uint        nfields;
} php_jit_union_t;

#define PHP_JIT_FETCH_UNION(from) \
	(php_jit_union_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_UNION_I(from) \
	(PHP_JIT_FETCH_UNION(from))->type

void php_jit_minit_union(int module_number TSRMLS_DC);

extern zend_function_entry php_jit_union_methods[];
extern zend_object_handlers php_jit_union_handlers;

#else
#ifndef HAVE_BITS_UNION
#define HAVE_BITS_UNION

zend_object_handlers php_jit_union_handlers;

static inline void php_jit_union_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_union_t *punion = 
		(php_jit_union_t *) zobject;
	zend_uint field = 0;
	
	zend_object_std_dtor(&punion->std TSRMLS_CC);
	
	while (field < punion->nfields) {
		zend_objects_store_del_ref_by_handle
			(punion->fields[field]->h TSRMLS_CC);
		field++;
	}
	
	jit_type_free(punion->type);
	
	efree(punion);
}

static inline zend_object_value php_jit_union_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value intern;
	php_jit_union_t *punion = 
		(php_jit_union_t*) emalloc(sizeof(php_jit_union_t));
	
	zend_object_std_init(&punion->std, ce TSRMLS_CC);
	object_properties_init(&punion->std, ce);
	
	punion->h = zend_objects_store_put(
		punion, 
		php_jit_union_destroy, NULL, NULL TSRMLS_CC);

	intern.handle   = punion->h;
	intern.handlers = &php_jit_union_handlers;
	
	return intern;
}

void php_jit_minit_union(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Union", php_jit_union_methods);
	jit_union_ce = zend_register_internal_class_ex(&ce, jit_type_ce, NULL TSRMLS_CC);
	jit_union_ce->create_object = php_jit_union_create;
	
	memcpy(
		&php_jit_union_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_union_handlers));
}

PHP_METHOD(Union, __construct) {
	zval **zztype;
	php_jit_union_t *punion;
	HashTable *zfields;
	HashPosition position;
	jit_type_t *fields;
	zend_uint   field = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "H", jit_type_ce, &zfields) != SUCCESS) {
		return;
	}
	
	punion = PHP_JIT_FETCH_UNION(getThis());
	
	punion->nfields = zend_hash_num_elements(zfields);
	punion->fields = (php_jit_type_t**) 
		ecalloc(punion->nfields, sizeof(php_jit_type_t));
	
	fields = (jit_type_t*)
		ecalloc(zend_hash_num_elements(zfields), sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(zfields, &position);
		zend_hash_get_current_data_ex(zfields, (void**)&zztype, &position) == SUCCESS;
		zend_hash_move_forward_ex(zfields, &position)) {
		if (instanceof_function(Z_OBJCE_PP(zztype), jit_type_ce TSRMLS_CC)) {
			punion->fields[field] = PHP_JIT_FETCH_TYPE(*zztype);
			zend_objects_store_add_ref_by_handle
				(punion->fields[field]->h TSRMLS_CC);
			fields[field] = PHP_JIT_FETCH_TYPE_I(*zztype);
		}
		field++;
	}

	punion->type = jit_type_create_union(fields, field, 1);
	efree(fields);
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_union_construct_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fields, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_union_methods[] = {
	PHP_ME(Struct, __construct,    php_jit_union_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
