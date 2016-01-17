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
#ifndef HAVE_BITS_TYPE_H
#define HAVE_BITS_TYPE_H

typedef struct _php_jit_type_t {
	jit_type_t          type;
	/* must be first three members */
	zend_ulong          id;
	zend_ulong          pt;
	zend_bool           copied;
	zend_object         std;
} php_jit_type_t;

zend_class_entry *jit_type_ce;

typedef struct _php_jit_sized_t {
	void       **data;
	int        length;
} php_jit_sized_t;

jit_type_t jit_type_sizable;
jit_type_t jit_type_sized;
jit_type_t jit_type_zobject;
jit_type_t jit_type_zvalue;
jit_type_t jit_type_zval;
jit_type_t jit_type_pzval;

#define PHP_JIT_FETCH_TYPE_O(o) ((php_jit_type_t*) ((char*) o - XtOffsetOf(php_jit_type_t, std)))
#define PHP_JIT_FETCH_TYPE(from) PHP_JIT_FETCH_TYPE_O(Z_OBJ_P(from))
#define PHP_JIT_FETCH_TYPE_I(from) (PHP_JIT_FETCH_TYPE(from))->type

#define PHP_JIT_TYPE_VOID  		1
#define PHP_JIT_TYPE_UINT       2
#define PHP_JIT_TYPE_INT        3
#define PHP_JIT_TYPE_ULONG      4
#define PHP_JIT_TYPE_LONG		5
#define PHP_JIT_TYPE_DOUBLE		6
#define PHP_JIT_TYPE_STRING		7
#define PHP_JIT_TYPE_ZVAL	    8

jit_type_t php_jit_type(short type);
void php_jit_minit_type(int module_number);

extern zend_function_entry php_jit_type_methods[];
extern zend_object_handlers php_jit_type_handlers;

jit_type_t jit_type_string;

#else
#ifndef HAVE_BITS_TYPE
#define HAVE_BITS_TYPE
zend_object_handlers php_jit_type_handlers;

jit_type_t php_jit_type(short type) {

	switch (type) {
		case PHP_JIT_TYPE_VOID:		return jit_type_void;
		case PHP_JIT_TYPE_UINT:     return jit_type_sys_uint;
		case PHP_JIT_TYPE_INT:      return jit_type_sys_int;
		case PHP_JIT_TYPE_ULONG:    return jit_type_sys_ulong;
		case PHP_JIT_TYPE_LONG:		return jit_type_sys_long;
		case PHP_JIT_TYPE_DOUBLE:	return jit_type_sys_double;
		case PHP_JIT_TYPE_STRING:   return jit_type_sized;
		case PHP_JIT_TYPE_ZVAL:	    return jit_type_pzval;
	}

	return jit_type_void;
}

static inline void php_jit_type_free(zend_object *zobject) {
	php_jit_type_t *ptype = 
		(php_jit_type_t *) PHP_JIT_FETCH_TYPE_O(zobject);

	if (ptype->copied) {
		jit_type_free(ptype->type);
	}
	
	zend_object_std_dtor(zobject);
}

static inline zend_object* php_jit_type_create(zend_class_entry *ce) {
	php_jit_type_t *ptype = 
		(php_jit_type_t*) ecalloc(1, sizeof(php_jit_type_t) + zend_object_properties_size(ce));
	
	zend_object_std_init(&ptype->std, ce);
	object_properties_init(&ptype->std, ce);
	
	ptype->std.handlers = &php_jit_type_handlers;
	
	return &ptype->std;
}

void php_jit_minit_type(int module_number) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Type", php_jit_type_methods);
	jit_type_ce = zend_register_internal_class(&ce);
	jit_type_ce->create_object = php_jit_type_create;
	
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("void"),    PHP_JIT_TYPE_VOID);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("int"),     PHP_JIT_TYPE_INT);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("uint"),    PHP_JIT_TYPE_UINT);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("ulong"),   PHP_JIT_TYPE_ULONG);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("long"),    PHP_JIT_TYPE_LONG);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("double"),  PHP_JIT_TYPE_DOUBLE);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("string"),  PHP_JIT_TYPE_STRING);
	zend_declare_class_constant_long(jit_type_ce, ZEND_STRL("zval"),    PHP_JIT_TYPE_ZVAL);
	
	memcpy(
		&php_jit_type_handlers,
		zend_get_std_object_handlers(),
		sizeof(php_jit_type_handlers));

	php_jit_type_handlers.offset = XtOffsetOf(php_jit_type_t, std);
	php_jit_type_handlers.free_obj = php_jit_type_free;

	REGISTER_LONG_CONSTANT("JIT_TYPE_VOID",      PHP_JIT_TYPE_VOID,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_INT",       PHP_JIT_TYPE_INT,         CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_UINT",      PHP_JIT_TYPE_UINT,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_LONG",      PHP_JIT_TYPE_LONG,        CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_ULONG",     PHP_JIT_TYPE_ULONG,       CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_DOUBLE",    PHP_JIT_TYPE_DOUBLE,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_STRING",    PHP_JIT_TYPE_STRING,      CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("JIT_TYPE_ZVAL",      PHP_JIT_TYPE_ZVAL,        CONST_CS|CONST_PERSISTENT);

	jit_type_string = jit_type_create_pointer(jit_type_sys_char, 0);
	{
		jit_type_t sFields[] = {
			jit_type_string,
			jit_type_sys_int
		};
		
		jit_type_sizable = jit_type_create_struct(sFields, sizeof(sFields)/sizeof(jit_type_t), 1);
		jit_type_sized   = jit_type_create_pointer(jit_type_sizable, 1);
		{
			jit_type_t oFields[] = {
				jit_type_sys_uint,
				jit_type_void_ptr
			};
			
			jit_type_zobject = jit_type_create_struct(oFields, sizeof(oFields)/sizeof(jit_type_t), 1);
			{
				jit_type_t vFields[] = {
					jit_type_sys_long,
					jit_type_sys_double,
					jit_type_sized,
					jit_type_void_ptr,
					jit_type_zobject,
					jit_type_void_ptr
				};

				jit_type_zvalue = jit_type_create_union(vFields, sizeof(vFields)/sizeof(jit_type_t), 1);
				{
					jit_type_t zFields[] = {
						jit_type_zvalue,
						jit_type_sys_uint,
						jit_type_sys_uchar,
						jit_type_sys_uchar
					};
					jit_type_zval = jit_type_create_struct(zFields, sizeof(zFields)/sizeof(jit_type_t), 1);
					jit_type_pzval = jit_type_create_pointer(jit_type_zval, 1);
				}
			}
		}
	}
}

PHP_METHOD(Type, __construct) {
	zval *ztype = NULL;
	zend_bool zpointer = 0;
	php_jit_type_t *ptype;
	
	if (php_jit_parameters("z|b", &ztype, &zpointer) != SUCCESS) {
		php_jit_exception("unexpected parameters, expected (int|Type of [, bool pointer = false])");
		return;
	}
	
	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	switch (Z_TYPE_P(ztype)) {
		case IS_LONG:
			if (zpointer) {
				ptype->type = jit_type_create_pointer(
					php_jit_type(Z_LVAL_P(ztype)), 1);
				ptype->pt   = 1;
			} else ptype->type = php_jit_type(Z_LVAL_P(ztype));
			ptype->id   = Z_LVAL_P(ztype);
		break;
		
		case IS_OBJECT: {
			php_jit_type_t *patype = PHP_JIT_FETCH_TYPE(ztype);
			if (zpointer) {
				ptype->type = jit_type_create_pointer(patype->type, 1);
				ptype->pt   = (patype->pt + 1);
			} else ptype->type = jit_type_copy(patype->type);
			ptype->id = patype->id;
			ptype->copied = 1;
		} break;
		
		default:
			php_jit_exception("unexpected parameters, expected (int|Type of [, bool pointer = false])");
	}
}

PHP_METHOD(Type, of) {
	long ztype = 0;
	zval *zcache = NULL;
	php_jit_type_t *ptype;
	
	if (php_jit_parameters("l", &ztype) != SUCCESS || !ztype) {
		php_jit_exception("unexpected parameters, expected (int type)");
		return;
	}
	
	if (!(zcache = zend_hash_index_find(&JG(types), ztype))) {
	 	object_init_ex(return_value, jit_type_ce);
	 	ptype = PHP_JIT_FETCH_TYPE(return_value);
	 	ptype->id = ztype;
	 	ptype->type = php_jit_type(ztype);
	 	zend_hash_index_update(
	 		&JG(types), ztype, return_value);
	 	Z_ADDREF_P(return_value);
	 } else ZVAL_COPY(return_value, zcache);
}

PHP_METHOD(Type, getIdentifier) {
	php_jit_type_t *ptype;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	RETURN_LONG(ptype->id);
}

PHP_METHOD(Type, getIndirection) {
	php_jit_type_t *ptype;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	RETURN_LONG(ptype->pt);
}

PHP_METHOD(Type, isPointer) {
	php_jit_type_t *ptype;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	RETURN_BOOL(ptype->pt > 0);
}

PHP_METHOD(Type, dump) {
	zval *zoutput = NULL;
	php_jit_type_t *ptype;
	php_stream *pstream = NULL;

	JIT_WIN32_NOT_IMPLEMENTED();

	if (php_jit_parameters("|r", &zoutput) != SUCCESS) {
		php_jit_exception("unexpected parameters, expected ([resource output = STDOUT])");
		return;
	}

	ptype = PHP_JIT_FETCH_TYPE(getThis());
	
	if (!zoutput) {
		jit_dump_type(stdout, ptype->type);
		return;
	}
	
	php_stream_from_zval(pstream, zoutput);
	
	if (php_stream_can_cast(pstream, PHP_STREAM_AS_STDIO|PHP_STREAM_CAST_TRY_HARD) == SUCCESS) {
		FILE *stdio;
		if (php_stream_cast(pstream, PHP_STREAM_AS_STDIO, (void**)&stdio, 0) == SUCCESS) {
			jit_dump_type(stdio, ptype->type);
		}
	}
}

ZEND_BEGIN_ARG_INFO_EX(php_jit_type_construct_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_type_of_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_type_dump_arginfo, 0, 0, 0) 
	ZEND_ARG_INFO(0, output)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_type_methods[] = {
	PHP_ME(Type, __construct,     php_jit_type_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Type, getIdentifier,   php_jit_no_arginfo,             ZEND_ACC_PUBLIC)
	PHP_ME(Type, getIndirection,  php_jit_no_arginfo,             ZEND_ACC_PUBLIC)
	PHP_ME(Type, isPointer,       php_jit_no_arginfo,             ZEND_ACC_PUBLIC)
	PHP_ME(Type, dump,            php_jit_type_dump_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Type, of,              php_jit_type_of_arginfo,        ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};
#endif
#endif
