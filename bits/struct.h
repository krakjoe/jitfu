/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 2014 Joe Watkins <krakjoe@php.net>                     |
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
#ifndef HAVE_BITS_STRUCT_H
#define HAVE_BITS_STRUCT_H

typedef struct _php_jit_struct_t {
	zend_object         std;
	zend_object_handle  h;
	jit_type_t          type;
	/* must be first three members */
	php_jit_type_t      **fields;
	char                **names;
	zend_uint             nfields;
} php_jit_struct_t;

zend_class_entry *jit_struct_ce;

#define PHP_JIT_FETCH_STRUCT(from) \
	(php_jit_struct_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_STRUCT_I(from) \
	(PHP_JIT_FETCH_STRUCT(from))->struct

void php_jit_minit_struct(int module_number TSRMLS_DC);

extern zend_function_entry php_jit_struct_methods[];
extern zend_object_handlers php_jit_struct_handlers;

#else
#ifndef HAVE_BITS_STRUCT
#define HAVE_BITS_STRUCT
zend_object_handlers php_jit_struct_handlers;

static inline void php_jit_struct_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_struct_t *pstruct = (php_jit_struct_t*) zobject;
	zend_uint nfield = 0;
		
	while (nfield < pstruct->nfields) {
		zend_objects_store_del_ref_by_handle
			(pstruct->fields[nfield]->h TSRMLS_CC);
		nfield++;
	}
	
	zend_objects_destroy_object(zobject, handle TSRMLS_CC);
}

static inline void php_jit_struct_free(void *zobject TSRMLS_DC) {
	php_jit_struct_t *pstruct = 
		(php_jit_struct_t *) zobject;

	zend_object_std_dtor(&pstruct->std TSRMLS_CC);
	
	jit_type_free(pstruct->type);
	
	if (pstruct->fields) {
		efree(pstruct->fields);
	}
	
	if (pstruct->names) {
		zend_uint nfield = 0;
		
		while (nfield < pstruct->nfields) {
			efree
				(pstruct->names[nfield]);
			nfield++;
		}
		
		efree(pstruct->names);
	}
	
	efree(pstruct);
}

static inline zend_object_value php_jit_struct_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value intern;
	php_jit_struct_t *pstruct = 
		(php_jit_struct_t*) ecalloc(1, sizeof(php_jit_struct_t));
	
	zend_object_std_init(&pstruct->std, ce TSRMLS_CC);
	object_properties_init(&pstruct->std, ce);
	
	pstruct->h = zend_objects_store_put(
		pstruct, 
		php_jit_struct_destroy, 
		php_jit_struct_free, NULL TSRMLS_CC);
	
	intern.handle   = pstruct->h;
	intern.handlers = &php_jit_struct_handlers;
	
	return intern;
}

void php_jit_minit_struct(int module_number TSRMLS_DC) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Struct", php_jit_struct_methods);
	jit_struct_ce = zend_register_internal_class_ex(&ce, jit_type_ce, NULL TSRMLS_CC);
	jit_struct_ce->create_object = php_jit_struct_create;

	memcpy(
		&php_jit_struct_handlers,
		zend_get_std_object_handlers(),
		sizeof(php_jit_struct_handlers));
}

PHP_METHOD(Struct, __construct) {
	HashTable *zfields;
	HashPosition zposition;
	php_jit_struct_t *pstruct;
	zval **zmember;
	zend_uint nfield = 0;
	jit_type_t *jfields;
	char **jnames = NULL;
	
	if (php_jit_parameters("H", &zfields) != SUCCESS) {
		php_jit_exception("unexpected parameters, expected (Type[] fields)");
		return;
	}
	
	pstruct = PHP_JIT_FETCH_STRUCT(getThis());
	
	pstruct->nfields = zend_hash_num_elements(zfields);
	pstruct->fields  = 
		(php_jit_type_t**) ecalloc(pstruct->nfields, sizeof(php_jit_struct_t*));
	jfields = 
		(jit_type_t*) ecalloc(pstruct->nfields, sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(zfields, &zposition);
		zend_hash_get_current_data_ex(zfields, (void**) &zmember, &zposition) == SUCCESS;
		zend_hash_move_forward_ex(zfields, &zposition)) {
		zend_ulong znidx = 0L;
		char *zname = NULL;
		int   znlength = 0;
		
		if (!zmember || 
			Z_TYPE_PP(zmember) != IS_OBJECT || 
			!instanceof_function(Z_OBJCE_PP(zmember), jit_type_ce TSRMLS_CC)) {
			php_jit_exception("non type found in fields list at %d", nfield);
			return;
		}
		
		pstruct->fields[nfield] = PHP_JIT_FETCH_TYPE(*zmember);
		zend_objects_store_add_ref_by_handle(pstruct->fields[nfield]->h TSRMLS_CC);
		jfields[nfield] = jit_type_copy(pstruct->fields[nfield]->type);
		
		if (zend_hash_get_current_key_ex(zfields, &zname, &znlength, &znidx, 0, &zposition) == HASH_KEY_IS_STRING) {
			if (!zname || !znlength) {
				php_jit_exception("invalid name found in fields list at %d", nfield);
				efree(jfields);
				return;
			}
			
			if (!pstruct->names) {
				pstruct->names = ecalloc(pstruct->nfields, sizeof(char*));
			}
			
			pstruct->names[nfield] = estrndup(zname, znlength);
		} else {
			if (pstruct->names) {
				php_jit_exception("un-named type found in fields list at %d", nfield);
				efree(jfields);
				return;
			}
		}
		
		nfield++;
	}
	
	pstruct->type = jit_type_create_struct(jfields, pstruct->nfields, 0);
	
	if (pstruct->names) {
		jit_type_set_names(pstruct->type, pstruct->names, pstruct->nfields);	
	}
	
	efree(jfields);
}

PHP_METHOD(Struct, getIdentifier) {
	php_jit_exception(
		"structs do not have identifiers");
}

PHP_METHOD(Struct, getIndirection) {
	RETURN_BOOL(0);
}

PHP_METHOD(Struct, isPointer) {
	RETURN_BOOL(0);
}

PHP_METHOD(Struct, getOffset) {
	zval *zoffset;
	php_jit_struct_t *pstruct;
	zend_uint of = 0;
	
	if (php_jit_parameters("z", &zoffset) != SUCCESS || !zoffset) {
		php_jit_exception("unexpected parameters, expected (string|integer field)");
		return;
	}
	
	pstruct = PHP_JIT_FETCH_STRUCT(getThis());
	
	switch (Z_TYPE_P(zoffset)) {
		case IS_STRING:
			if (!pstruct->names) {
				php_jit_exception("no names available");
			}
			
			of = jit_type_find_name
				(pstruct->type, Z_STRVAL_P(zoffset));
		break;
		
		case IS_LONG:	
			of = Z_LVAL_P(zoffset);
		break;
		
		default:
			php_jit_exception("unexpected parameters, expected (string|integer field)");
			return;
	}
	
	if (of < pstruct->nfields) {
		RETURN_LONG(jit_type_get_offset(pstruct->type, of));
	}
	
	php_jit_exception("failed to find offset of requested field");
}

PHP_METHOD(Struct, getFieldType) {
	zval *zoffset;
	php_jit_struct_t *pstruct;
	zend_uint of = 0;
	
	if (php_jit_parameters("z", &zoffset) != SUCCESS || !zoffset) {
		php_jit_exception("unexpected parameters, expected (string|integer field)");
		return;
	}
	
	pstruct = PHP_JIT_FETCH_STRUCT(getThis());
	
	switch (Z_TYPE_P(zoffset)) {
		case IS_STRING:
			if (!pstruct->names) {
				php_jit_exception("no names available");
				return;
			}
			
			of = jit_type_find_name
				(pstruct->type, Z_STRVAL_P(zoffset));
		break;
		
		case IS_LONG: of = Z_LVAL_P(zoffset); break;
		
		default:
			php_jit_exception("unexpected parameters, expected (string|integer field)");
			return;
	}
	
	if (of < pstruct->nfields) {
		Z_OBJ_HANDLE_P(return_value) = pstruct->fields[of]->h;
		Z_OBJ_HT_P(return_value)     = &php_jit_type_handlers;
		Z_TYPE_P(return_value)       = IS_OBJECT;
	
		zend_objects_store_add_ref_by_handle(pstruct->fields[of]->h TSRMLS_CC);
		return;
	}
	
	php_jit_exception("failed to find the requested field");
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_struct_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_struct_getOffset_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_struct_getFieldType_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, field)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_struct_methods[] = {
	PHP_ME(Struct, __construct,     php_jit_struct_construct_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Struct, getIdentifier,   php_jit_no_arginfo,                   ZEND_ACC_PUBLIC)
	PHP_ME(Struct, getIndirection,  php_jit_no_arginfo,                   ZEND_ACC_PUBLIC)
	PHP_ME(Struct, isPointer,       php_jit_no_arginfo,                   ZEND_ACC_PUBLIC)
	PHP_ME(Struct, getOffset,       php_jit_struct_getOffset_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Struct, getFieldType,    php_jit_struct_getFieldType_arginfo,  ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
