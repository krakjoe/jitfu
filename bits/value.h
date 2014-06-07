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
#ifndef HAVE_BITS_VALUE_H
#define HAVE_BITS_VALUE_H

typedef struct _php_jit_value_t {
	zend_object std;
	zend_object_handle h;
	php_jit_function_t *func;
	php_jit_type_t     *type;
	jit_value_t         value;
	void               *dup;
} php_jit_value_t;

zend_class_entry *jit_value_ce;

void php_jit_minit_value(int module_number TSRMLS_DC);

#define PHP_JIT_FETCH_VALUE(from) \
	(php_jit_value_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_VALUE_I(from) \
	(PHP_JIT_FETCH_VALUE(from))->value

extern zend_function_entry php_jit_value_methods[];
extern zend_object_handlers php_jit_value_handlers;

#else
#ifndef HAVE_BITS_VALUE
#define HAVE_BITS_VALUE
zend_object_handlers php_jit_value_handlers;

static inline void php_jit_value_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_value_t *pval = 
		(php_jit_value_t *) zobject;

	zend_object_std_dtor(&pval->std TSRMLS_CC);

	if (pval->func) {
		zend_objects_store_del_ref_by_handle(pval->func->h TSRMLS_CC);
	}
	
	if (pval->type) {
		zend_objects_store_del_ref_by_handle(pval->type->h TSRMLS_CC);
	}

	if (pval->dup) {
		efree(pval->dup);
	}
	
	efree(pval);
}

static inline zend_object_value php_jit_value_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value value;
	php_jit_value_t *pval = 
		(php_jit_value_t*) ecalloc(1, sizeof(php_jit_value_t));
	
	zend_object_std_init(&pval->std, ce TSRMLS_CC);
	object_properties_init(&pval->std, ce);
	
	pval->h = zend_objects_store_put(
		pval, 
		php_jit_value_destroy, NULL, NULL TSRMLS_CC);

	value.handle   = pval->h;
	value.handlers = &php_jit_value_handlers;
	
	return value;
}

void php_jit_minit_value(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Value", php_jit_value_methods);
	jit_value_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_value_ce->create_object = php_jit_value_create;
	
	memcpy(
		&php_jit_value_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_value_handlers));
}

PHP_METHOD(Value, __construct) {
	zval *zfunction = NULL, 
		 *zvalue = NULL,
		 *ztype = NULL;
	php_jit_value_t *pval;
	
	switch (ZEND_NUM_ARGS()) {
		case 3: if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OzO", &zfunction, jit_function_ce, &zvalue, &ztype, jit_type_ce) != SUCCESS) {
			return;
		} break;
		
		default: if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zfunction, jit_function_ce, &ztype, jit_type_ce) != SUCCESS) {
			return;
		}
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	pval->func = PHP_JIT_FETCH_FUNCTION(zfunction);
	zend_objects_store_add_ref_by_handle
		(pval->func->h TSRMLS_CC);
	
	pval->type = PHP_JIT_FETCH_TYPE(ztype);
	zend_objects_store_add_ref_by_handle
		(pval->type->h TSRMLS_CC);
	
	if (zvalue) {
		switch (Z_TYPE_P(zvalue)) {
			case IS_LONG:
				if (pval->type) {
					pval->value = jit_value_create_long_constant(pval->func->func, pval->type->type, Z_LVAL_P(zvalue));	
				} else pval->value = jit_value_create_long_constant(pval->func->func, jit_type_sys_long, Z_LVAL_P(zvalue));
			break;
		
			case IS_DOUBLE:
				if (pval->type) {
					pval->value = jit_value_create_nfloat_constant(pval->func->func, pval->type->type, Z_DVAL_P(zvalue));
				} else pval->value = jit_value_create_nfloat_constant(pval->func->func, jit_type_sys_double, Z_DVAL_P(zvalue));
			break;
		
			default: {
				jit_constant_t con;

				con.un.ptr_value = &zvalue->value;
				con.type         = pval->type->type;

				pval->value = jit_value_create_constant(pval->func->func, &con);
			} break;
		}
	} else {
		pval->value = jit_value_create(pval->func->func, pval->type->type);
	}
}

PHP_METHOD(Value, isTemporary) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	RETURN_BOOL(jit_value_is_temporary(pval->value));
}

PHP_METHOD(Value, isLocal) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	RETURN_BOOL(jit_value_is_local(pval->value));
}

PHP_METHOD(Value, isConstant) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	RETURN_BOOL(jit_value_is_constant(pval->value));
}

PHP_METHOD(Value, isParameter) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	RETURN_BOOL(jit_value_is_parameter(pval->value));
}

PHP_METHOD(Value, isVolatile) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	RETURN_BOOL(jit_value_is_volatile(pval->value));
}

PHP_METHOD(Value, isAddressable) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	RETURN_BOOL(jit_value_is_addressable(pval->value));
}

PHP_METHOD(Value, isTrue) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	RETURN_BOOL(jit_value_is_true(pval->value));
}

PHP_METHOD(Value, setVolatile) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	jit_value_set_volatile(pval->value);
}

PHP_METHOD(Value, setAddressable) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	jit_value_set_addressable(pval->value);
}

PHP_METHOD(Value, getType) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	if (pval) {
		zend_object_value value;
		
		value.handle = pval->type->h;
		value.handlers = &php_jit_type_handlers;
		
		Z_TYPE_P(return_value)   = IS_OBJECT;
		Z_OBJVAL_P(return_value) = value;
		
		zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
	}
}

PHP_METHOD(Value, getFunction) {
	php_jit_value_t *pval;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	if (pval) {
		zend_object_value value;
		
		value.handle = pval->func->h;
		value.handlers = &php_jit_function_handlers;
		
		Z_TYPE_P(return_value)   = IS_OBJECT;
		Z_OBJVAL_P(return_value) = value;
		
		zend_objects_store_add_ref_by_handle(pval->func->h TSRMLS_CC);
	}
}

PHP_METHOD(Value, dump) {
	zval *zoutput = NULL, *zprefix = NULL;
	php_jit_value_t *pval;
	php_stream *pstream = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zz", &zoutput, &zprefix) != SUCCESS) {
		return;
	}

	pval = PHP_JIT_FETCH_VALUE(getThis());
	
	if (!zoutput) {
		jit_dump_value(stdout, 
			jit_value_get_function(pval->value), 
			pval->value, zprefix ? Z_STRVAL_P(zprefix) : NULL);
		return;
	}
	
	php_stream_from_zval(pstream, &zoutput);

	if (php_stream_can_cast(pstream, PHP_STREAM_AS_STDIO|PHP_STREAM_CAST_TRY_HARD) == SUCCESS) {
		FILE *stdio;
		if (php_stream_cast(pstream, PHP_STREAM_AS_STDIO, (void**)&stdio, 0) == SUCCESS) {
			jit_dump_value(stdio, 
				jit_value_get_function(pval->value), 
				pval->value, zprefix ? Z_STRVAL_P(zprefix) : NULL);
		}
	}
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_value_construct_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_value_dump_arginfo, 0, 0, 0) 
	ZEND_ARG_INFO(0, output)
	ZEND_ARG_INFO(0, prefix)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_value_methods[] = {
	PHP_ME(Value, __construct,    php_jit_value_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, isTemporary,    php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, isLocal,        php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, isConstant,     php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, isParameter,    php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, isVolatile,     php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, isAddressable,  php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, isTrue,         php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, setVolatile,    php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, setAddressable, php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, getType,        php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, getFunction,    php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Value, dump,           php_jit_value_dump_arginfo, ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
