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
	jit_type_t          type;
	zval                *zfields;
	zend_string         **names;
	zend_ulong          nfields;
	zend_object         std;
} php_jit_struct_t;

zend_class_entry *jit_struct_ce;

#define PHP_JIT_FETCH_STRUCT_O(o) ((php_jit_struct_t*) ((char*) o - XtOffsetOf(php_jit_struct_t, std)))
#define PHP_JIT_FETCH_STRUCT(from) PHP_JIT_FETCH_STRUCT_O(Z_OBJ_P(from))
#define PHP_JIT_FETCH_STRUCT_I(from) (PHP_JIT_FETCH_STRUCT(from))->struct

void php_jit_minit_struct(int module_number);

extern zend_function_entry php_jit_struct_methods[];
extern zend_object_handlers php_jit_struct_handlers;

#else
#ifndef HAVE_BITS_STRUCT
#define HAVE_BITS_STRUCT
zend_object_handlers php_jit_struct_handlers;

static inline void php_jit_struct_free(zend_object *object) {
	php_jit_struct_t *pstruct = 
		(php_jit_struct_t *) PHP_JIT_FETCH_STRUCT_O(object);
	zend_ulong nfield = 0;
		
	while (nfield < pstruct->nfields) {
		zval_ptr_dtor(&pstruct->zfields[nfield]);
		nfield++;
	}
	
	jit_type_free(pstruct->type);
	
	if (pstruct->zfields) {
		efree(pstruct->zfields);
	}
	
	if (pstruct->names) {
		zend_ulong nfield = 0;
		
		while (nfield < pstruct->nfields) {
			zend_string_release
				(pstruct->names[nfield]);
			nfield++;
		}
		
		efree(pstruct->names);
	}

	zend_object_std_dtor(&pstruct->std);
}

static inline zend_object* php_jit_struct_create(zend_class_entry *ce) {
	php_jit_struct_t *pstruct = 
		(php_jit_struct_t*) ecalloc(1, sizeof(php_jit_struct_t) + zend_object_properties_size(ce));
	
	zend_object_std_init(&pstruct->std, ce);
	object_properties_init(&pstruct->std, ce);
	
	pstruct->std.handlers = &php_jit_struct_handlers;
	
	return &pstruct->std;
}

void php_jit_minit_struct(int module_number) {
	zend_class_entry ce;

	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Struct", php_jit_struct_methods);
	jit_struct_ce = zend_register_internal_class_ex(&ce, jit_type_ce);
	jit_struct_ce->create_object = php_jit_struct_create;

	memcpy(
		&php_jit_struct_handlers,
		zend_get_std_object_handlers(),
		sizeof(php_jit_struct_handlers));

	php_jit_struct_handlers.offset = XtOffsetOf(php_jit_struct_t, std);
	php_jit_struct_handlers.free_obj = php_jit_struct_free;
}

PHP_METHOD(Struct, __construct) {
	HashTable *zfields;
	HashPosition zposition;
	php_jit_struct_t *pstruct;
	zval *zmember;
	zend_ulong nfield = 0;
	jit_type_t *jfields;
	char **jnames = NULL;
	
	if (php_jit_parameters("H", &zfields) != SUCCESS) {
		php_jit_exception("unexpected parameters, expected (Type[] fields)");
		return;
	}
	
	pstruct = PHP_JIT_FETCH_STRUCT(getThis());
	
	pstruct->nfields = zend_hash_num_elements(zfields);
	pstruct->zfields  = 
		(zval*) ecalloc(pstruct->nfields, sizeof(zval));
	jfields = 
		(jit_type_t*) ecalloc(pstruct->nfields, sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(zfields, &zposition);
		(zmember = zend_hash_get_current_data_ex(zfields, &zposition));
		zend_hash_move_forward_ex(zfields, &zposition)) {
		zend_ulong znidx = 0L;
		zend_string *zname = NULL;
		php_jit_type_t *ptype;
		
		if (!zmember || 
			Z_TYPE_P(zmember) != IS_OBJECT || 
			!instanceof_function(Z_OBJCE_P(zmember), jit_type_ce)) {
			php_jit_exception("non type found in fields list at %d", nfield);
			return;
		}
		ZVAL_COPY(&pstruct->zfields[nfield], zmember);
		
		ptype = 
		    PHP_JIT_FETCH_TYPE(&pstruct->zfields[nfield]);
		
		jfields[nfield] = jit_type_copy(ptype->type);
		
		if (zend_hash_get_current_key_ex(zfields, &zname, &znidx, &zposition) == HASH_KEY_IS_STRING) {
			if (!zname || !ZSTR_LEN(zname)) {
				php_jit_exception("invalid name found in fields list at %d", nfield);
				efree(jfields);
				return;
			}
			
			if (!pstruct->names) {
				pstruct->names = ecalloc(pstruct->nfields, sizeof(zend_string*));
			}
			
			pstruct->names[nfield] = zend_string_copy(zname);
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
		jit_type_set_names(pstruct->type, (char**) pstruct->names, pstruct->nfields);	
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
	zend_ulong of = 0;
	
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
	zend_ulong of = 0;
	
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
	    ZVAL_COPY_VALUE(return_value, &pstruct->zfields[of]);
	    zval_copy_ctor(return_value);
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
