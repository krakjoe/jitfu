#ifndef HAVE_BITS_TYPE_H
#define HAVE_BITS_TYPE_H

#define PHP_JIT_TYPE_FUNCTIONS \
	JIT_FE(jit_type_copy) \
	JIT_FE(jit_type_free) \
	JIT_FE(jit_type_create_struct) \
	JIT_FE(jit_type_create_union) \
	JIT_FE(jit_type_create_signature) \
	JIT_FE(jit_type_create_pointer)

static const char *le_jit_type_name = "jit type";
static       int   le_jit_type;

ZEND_RSRC_DTOR_FUNC(php_jit_type_dtor);

static inline php_jit_minit_type(int module_number TSRMLS_DC) {
	le_jit_type = zend_register_list_destructors_ex
		(php_jit_type_dtor, NULL, le_jit_type_name, module_number);

	REGISTER_LONG_CONSTANT("JIT_ABI_CDECL",      (zend_ulong) jit_abi_cdecl,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_ABI_VARARG",     (zend_ulong) jit_abi_vararg,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_ABI_STDCALL",    (zend_ulong) jit_abi_stdcall,    CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_ABI_FASTCALL",   (zend_ulong) jit_abi_fastcall,   CONST_CS|CONST_PERSISTENT);	

	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID",      (zend_ulong) jit_type_void,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_SBYTE",     (zend_ulong) jit_type_sbyte,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_UBYTE",     (zend_ulong) jit_type_ubyte,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_SHORT",     (zend_ulong) jit_type_short,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_USHORT",    (zend_ulong) jit_type_ushort,    CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_INT",       (zend_ulong) jit_type_int,       CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_UINT",      (zend_ulong) jit_type_uint,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_NINT",      (zend_ulong) jit_type_nint,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_NUINT",     (zend_ulong) jit_type_nuint,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_LONG",      (zend_ulong) jit_type_long,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_ULONG",     (zend_ulong) jit_type_ulong,     CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_FLOAT32",   (zend_ulong) jit_type_float32,   CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_FLOAT64",   (zend_ulong) jit_type_float64,   CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_NFLOAT",    (zend_ulong) jit_type_nfloat,    CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID_PTR",  (zend_ulong) jit_type_void_ptr,  CONST_CS|CONST_PERSISTENT);
}

ZEND_BEGIN_ARG_INFO_EX(jit_type_copy_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_free_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_struct_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fields, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, incref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_union_arginfo, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, fields, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, incref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_signature_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, abi)
	ZEND_ARG_INFO(0, returns)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
	ZEND_ARG_INFO(0, incref)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(jit_type_create_pointer_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, incref)
ZEND_END_ARG_INFO()

PHP_FUNCTION(jit_type_copy);
PHP_FUNCTION(jit_type_free);
PHP_FUNCTION(jit_type_create_struct);
PHP_FUNCTION(jit_type_create_union);
PHP_FUNCTION(jit_type_create_signature);
PHP_FUNCTION(jit_type_create_pointer);
#else
#ifndef HAVE_BITS_TYPE
#define HAVE_BITS_TYPE

/* {{{ php_jit_type_dtor */
ZEND_RSRC_DTOR_FUNC(php_jit_type_dtor) {
	//jit_type_free((jit_type_t) rsrc->ptr);
} /* }}} */

/* {{{ jit_type_t jit_type_copy(jit_type_t type) */
PHP_FUNCTION(jit_type_copy) {
	zval *resource;
	jit_type_t type, copy;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(type, jit_type_t, &resource, -1, le_jit_type_name, le_jit_type);
	
	copy = jit_type_copy(type);
	
	ZEND_REGISTER_RESOURCE(return_value, copy, le_jit_type);
} /* }}} */

/* {{{ void jit_type_free(jit_type_t type) */
PHP_FUNCTION(jit_type_free) {
	zval *resource;
	jit_type_t type;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(type, jit_type_t, &resource, -1, le_jit_type_name, le_jit_type);
	
	zend_list_del(Z_RESVAL_P(resource));
} /* }}} */

/* {{{ jit_type_t jit_type_create_struct(array fields [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_struct) {
	zval       **field;
	HashTable   *fields;
	long         incref = 0;
	HashPosition position;
	jit_type_t  *jfields;
	jit_type_t   structure;
	zend_uint    arg = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Hl", &fields, &incref) != SUCCESS) {
		return;
	}
	
	jfields = (jit_type_t*) ecalloc(zend_hash_num_elements(fields), sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(fields, &position);	
		zend_hash_get_current_data_ex(fields, (void**) &field, &position) == SUCCESS;
		zend_hash_move_forward_ex(fields, &position)) {
		
		switch (Z_TYPE_PP(field)) {
			case IS_LONG: 
				jfields[arg] = (jit_type_t) Z_LVAL_PP(field); 
			break;
			
			case IS_RESOURCE: {
				jit_type_t res;
				ZEND_FETCH_RESOURCE(res, jit_type_t, field, -1, le_jit_type_name, le_jit_type);
				jfields[arg] = jit_type_copy(res);
			} break;
		}
		arg++;
	}
	
	structure = jit_type_create_struct(jfields, zend_hash_num_elements(fields), incref);
	
	ZEND_REGISTER_RESOURCE(return_value, structure, le_jit_type);
} /* }}} */

/* {{{ jit_type_t jit_type_create_union(array fields [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_union) {
	zval       **field;
	HashTable   *fields;
	HashPosition position;
	jit_type_t  *jfields;
	jit_type_t   structure;
	zend_uint    arg = 0;
	long         incref = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Hl", &fields, &incref) != SUCCESS) {
		return;
	}
	
	jfields = (jit_type_t*) ecalloc(zend_hash_num_elements(fields), sizeof(jit_type_t));
	
	for (zend_hash_internal_pointer_reset_ex(fields, &position);	
		zend_hash_get_current_data_ex(fields, (void**) &field, &position) == SUCCESS;
		zend_hash_move_forward_ex(fields, &position)) {
		
		switch (Z_TYPE_PP(field)) {
			case IS_LONG: 
				jfields[arg] = (jit_type_t) Z_LVAL_PP(field); 
			break;
			
			case IS_RESOURCE: {
				jit_type_t res;
				ZEND_FETCH_RESOURCE(res, jit_type_t, field, -1, le_jit_type_name, le_jit_type);
				jfields[arg] = jit_type_copy(res);
			} break;
		}
		arg++;
	}
	
	structure = jit_type_create_union(jfields, zend_hash_num_elements(fields), incref);
	
	ZEND_REGISTER_RESOURCE(return_value, structure, le_jit_type);
} /* }}} */

/* {{{ jit_type_t jit_type_create_signature(int abi, mixed returns, array params [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_signature) 
{
	long         abi;
	zval       **param, *returns;
	HashTable   *params;
	HashPosition position;
	jit_type_t  *jparams;
	jit_type_t   signature;
	jit_type_t   jreturns;
	zend_uint    jparam = 0;
	long         incref = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lzH|l", &abi, &returns, &params, &incref) != SUCCESS) {
		return;
	}
	
	jparams = (jit_type_t*) calloc(zend_hash_num_elements(params), sizeof(jit_type_t));
	
	if (Z_TYPE_P(returns) == IS_LONG) {
		jreturns = (jit_type_t) jreturns;
	} else if (Z_TYPE_P(returns) == IS_RESOURCE) {
		ZEND_FETCH_RESOURCE(jreturns, jit_type_t, &returns, -1, le_jit_type_name, le_jit_type);
	}
	
	for (zend_hash_internal_pointer_reset_ex(params, &position);	
		zend_hash_get_current_data_ex(params, (void**) &param, &position) == SUCCESS;
		zend_hash_move_forward_ex(params, &position)) {
		
		switch (Z_TYPE_PP(param)) {
			case IS_LONG: 
				jparams[jparam] = (jit_type_t) Z_LVAL_PP(param); 
			break;
			
			case IS_RESOURCE: {
				jit_type_t res;
				ZEND_FETCH_RESOURCE(res, jit_type_t, param, -1, le_jit_type_name, le_jit_type);
				jparams[jparam] = jit_type_copy(res);
			} break;
		}
		jparam++;
	}
	
	signature = jit_type_create_signature(
		abi, jreturns, jparams, zend_hash_num_elements(params), incref);
	
	ZEND_REGISTER_RESOURCE(return_value, signature, le_jit_type);
} /* }}} */

/* {{{ jit_type_t jit_type_create_pointer(jit_type_t type [, int incref = 0]) */
PHP_FUNCTION(jit_type_create_pointer) {
	zval *resource;
	jit_type_t type, pointer;
	long incref = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|l", &resource, &incref) != SUCCESS) {
		return;
	}
	
	ZEND_FETCH_RESOURCE(type, jit_type_t, &resource, -1, le_jit_type_name, le_jit_type);
	
	pointer = jit_type_create_pointer(type, incref);
	
	ZEND_REGISTER_RESOURCE(return_value, pointer, le_jit_type);
} /* }}} */
#endif
#endif
