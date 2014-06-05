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
#ifndef HAVE_BITS_FUNCTION_H
#define HAVE_BITS_FUNCTION_H

typedef struct _php_jit_function_t php_jit_function_t;
struct _php_jit_function_t {
	zend_object         std;
	zend_object_handle  h;
	php_jit_context_t   *ctx;
	php_jit_signature_t *sig;
	php_jit_function_t  *parent;
	jit_function_t      func;
	zend_ulong          st;
};

zend_class_entry *jit_function_ce;

void php_jit_minit_function(int module_number TSRMLS_DC);

#define PHP_JIT_FETCH_FUNCTION(from) \
	(php_jit_function_t*) zend_object_store_get_object((from) TSRMLS_CC)
#define PHP_JIT_FETCH_FUNCTION_I(from) \
	(PHP_JIT_FETCH_FUNCTION(from))->func
	
#define PHP_JIT_FUNCTION_CREATED  (1<<1)
#define PHP_JIT_FUNCTION_COMPILED (1<<2)

extern zend_function_entry php_jit_function_methods[];
extern zend_object_handlers php_jit_function_handlers;

#else
#ifndef HAVE_BITS_FUNCTION
#define HAVE_BITS_FUNCTION
zend_object_handlers php_jit_function_handlers;

static inline void php_jit_function_destroy(void *zobject, zend_object_handle handle TSRMLS_DC) {
	php_jit_function_t *pfunc = 
		(php_jit_function_t *) zobject;

	zend_object_std_dtor(&pfunc->std TSRMLS_CC);

	zend_objects_store_del_ref_by_handle(pfunc->ctx->h TSRMLS_CC);
	zend_objects_store_del_ref_by_handle(pfunc->sig->h TSRMLS_CC);
	
	if (pfunc->parent) {
		zend_objects_store_del_ref_by_handle(pfunc->parent->h TSRMLS_CC);
	}
	
	efree(pfunc);
}

static inline zend_object_value php_jit_function_create(zend_class_entry *ce TSRMLS_DC) {
	zend_object_value value;
	php_jit_function_t *pfunc = 
		(php_jit_function_t*) ecalloc(1, sizeof(php_jit_function_t));
	
	zend_object_std_init(&pfunc->std, ce TSRMLS_CC);
	object_properties_init(&pfunc->std, ce);
	
	pfunc->h = zend_objects_store_put(
		pfunc, 
		php_jit_function_destroy, NULL, NULL TSRMLS_CC);

	value.handle   = pfunc->h;
	value.handlers = &php_jit_function_handlers;
	
	return value;
}

void php_jit_minit_function(int module_number TSRMLS_DC) {
	zend_class_entry ce;
	
	INIT_NS_CLASS_ENTRY(ce, "JITFU", "Func", php_jit_function_methods);
	jit_function_ce = zend_register_internal_class(&ce TSRMLS_CC);
	jit_function_ce->create_object = php_jit_function_create;
	
	memcpy(
		&php_jit_function_handlers,
		zend_get_std_object_handlers(), 
		sizeof(php_jit_function_handlers));
}

PHP_METHOD(Func, __construct) {
	zval *zcontext;
	zval *zsignature;
	zval *zparent = NULL;
	
	php_jit_function_t *pfunc;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO|O", &zcontext, jit_context_ce, &zsignature, jit_signature_ce, &zparent, jit_function_ce) != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	pfunc->ctx = PHP_JIT_FETCH_CONTEXT(zcontext);
	zend_objects_store_add_ref_by_handle
		(pfunc->ctx->h TSRMLS_CC);
	
	if (!(pfunc->ctx->st & PHP_JIT_CONTEXT_STARTED)) {
		
		jit_context_build_start(pfunc->ctx->ctx);
		
		pfunc->ctx->st |= PHP_JIT_CONTEXT_STARTED;
	}
	
	pfunc->sig = PHP_JIT_FETCH_SIGNATURE(zsignature);
	zend_objects_store_add_ref_by_handle
		(pfunc->sig->h TSRMLS_CC);
	
	if (zparent) {
		pfunc->parent = PHP_JIT_FETCH_FUNCTION(zparent);
		zend_objects_store_add_ref_by_handle
			(pfunc->parent->h TSRMLS_CC);
	}
	
	if (!zparent) {
		pfunc->func = jit_function_create
			(pfunc->ctx->ctx, pfunc->sig->type);
	} else {
		pfunc->func = jit_function_create_nested
			(pfunc->ctx->ctx, pfunc->sig->type, pfunc->parent->func);
	}

	pfunc->st |= PHP_JIT_FUNCTION_CREATED;
}

PHP_METHOD(Func, compile) {
	php_jit_function_t *pfunc;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	if (pfunc->st & PHP_JIT_FUNCTION_COMPILED) {
		zend_throw_exception_ex(NULL, 0 TSRMLS_CC, 
			"this function was already compiled");
		return;
	}
	
	jit_function_compile(pfunc->func);
	
	pfunc->st |= PHP_JIT_FUNCTION_COMPILED;
}

PHP_METHOD(Func, isCompiled) {
	php_jit_function_t *pfunc;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	RETURN_BOOL(pfunc->st & PHP_JIT_FUNCTION_COMPILED);
}

PHP_METHOD(Func, isNested) {
	php_jit_function_t *pfunc;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	RETURN_BOOL(pfunc->parent != NULL);
}

PHP_METHOD(Func, getParent) {
	php_jit_function_t *pfunc;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	if (pfunc->parent) {
		zend_object_value value;
		
		value.handle   = pfunc->parent->h;
		value.handlers = &php_jit_function_handlers;
		
		Z_TYPE_P(return_value)   = IS_OBJECT;
		Z_OBJVAL_P(return_value) = value;
		
		zend_objects_store_add_ref_by_handle(pfunc->parent->h TSRMLS_CC);
	}
}

PHP_METHOD(Func, getContext) {
	php_jit_function_t *pfunc;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	if (pfunc->ctx) {
		zend_object_value value;

		value.handle   = pfunc->ctx->h;
		value.handlers = &php_jit_context_handlers;
	
		Z_TYPE_P(return_value)   = IS_OBJECT;
		Z_OBJVAL_P(return_value) = value;
	
		zend_objects_store_add_ref_by_handle(pfunc->ctx->h TSRMLS_CC);
	}
}

PHP_METHOD(Func, getSignature) {
	php_jit_function_t *pfunc;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	if (pfunc->sig) {
		zend_object_value value;

		value.handle   = pfunc->sig->h;
		value.handlers = &php_jit_signature_handlers;

		Z_TYPE_P(return_value)   = IS_OBJECT;
		Z_OBJVAL_P(return_value) = value;

		zend_objects_store_add_ref_by_handle(pfunc->sig->h TSRMLS_CC);
	}
}

static inline void** php_jit_array_args(php_jit_function_t *pfunc, zend_llist *stack, zval *member, zend_uint narg TSRMLS_DC) {
	HashTable *uht = Z_ARRVAL_P(member);
	HashPosition pos;
	zend_uint nuargs = zend_hash_num_elements(uht), 
			  nuarg = 0;
	void **uargs = NULL;
	void **pargs = NULL;
	zval **zmember;

#define PHP_JIT_INIT_ARGS(type) do { \
	\
	uargs = emalloc(sizeof(type) * nuargs); \
	pargs = emalloc(sizeof(type*) * nuargs); \
} while (0)

	switch (pfunc->sig->params[narg]->id) {
		case PHP_JIT_TYPE_INT:    PHP_JIT_INIT_ARGS(int);     break;
		case PHP_JIT_TYPE_UINT:   PHP_JIT_INIT_ARGS(uint);    break;
		case PHP_JIT_TYPE_LONG:   PHP_JIT_INIT_ARGS(long);    break;
		case PHP_JIT_TYPE_ULONG:  PHP_JIT_INIT_ARGS(ulong);   break;
		case PHP_JIT_TYPE_DOUBLE: PHP_JIT_INIT_ARGS(double);  break;
		case PHP_JIT_TYPE_STRING: PHP_JIT_INIT_ARGS(char*);   break;
		case PHP_JIT_TYPE_VOID_PTR: PHP_JIT_INIT_ARGS(void*); break;
		
		default: {
			/* throw cannot manage arguments */
			efree(uargs);
			efree(pargs);
			return;
		}
	}

#undef PHP_JIT_INIT_ARGS

	zend_llist_add_element(stack, &uargs);
	zend_llist_add_element(stack, &pargs);

	for (zend_hash_internal_pointer_reset_ex(uht, &pos);
		zend_hash_get_current_data_ex(uht, (void**) &zmember, &pos) == SUCCESS;
		zend_hash_move_forward_ex(uht, &pos)) {
		
		if (Z_TYPE_PP(zmember) == IS_ARRAY) {
			if (!pfunc->sig->params[narg]->pt) {
				/* throw, this is not a pointer type */
				efree(pargs);
				efree(uargs);
				return NULL;
			}
			
			((void**)uargs)[nuarg] = * php_jit_array_args
				(pfunc, stack, *zmember, narg TSRMLS_CC);
			
		} else switch (pfunc->sig->params[narg]->id) {
			case PHP_JIT_TYPE_LONG:   ((long*)uargs)[nuarg]   = Z_LVAL_PP(zmember);           break;
			case PHP_JIT_TYPE_ULONG:  ((ulong*)uargs)[nuarg]  = (ulong) Z_LVAL_PP(zmember);   break;
			case PHP_JIT_TYPE_INT:    ((int*)uargs)[nuarg]    = (int) Z_LVAL_PP(zmember);     break;
			case PHP_JIT_TYPE_UINT:   ((uint*)uargs)[nuarg]   = (uint) Z_LVAL_PP(zmember);    break;
			case PHP_JIT_TYPE_DOUBLE: ((double*)uargs)[nuarg] = Z_DVAL_PP(zmember);           break;
			case PHP_JIT_TYPE_STRING: ((char**)uargs)[nuarg]  = Z_STRVAL_PP(zmember);         break;
			case PHP_JIT_TYPE_VOID_PTR: ((void**)uargs)[nuarg] = &(*zmember)->value;          break;
		}
		
		pargs[nuarg] = &uargs[nuarg];
		nuarg++;
	}
	
	return pargs;
}

static inline void php_jit_invoke_stack_dtor(void *ptr) {
	efree(*(void**)ptr);
}

PHP_METHOD(Func, __invoke) {
	php_jit_function_t *pfunc;
	
	zend_uint nargs = ZEND_NUM_ARGS();
	zval **args = nargs ?
		(zval**) emalloc(sizeof(zval*) * nargs) : NULL;
	void **jargs = nargs ?
		(void**) emalloc(sizeof(void*) * nargs) : NULL;
	void *result;
	zend_llist stack;
		
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	/* TODO(krakjoe) this requires more thought and work ... */
	
	if (!jit_function_is_compiled(pfunc->func)) {
		if (!(pfunc->ctx->st & PHP_JIT_CONTEXT_FINISHED)) {
			jit_context_build_end(pfunc->ctx->ctx);
			pfunc->ctx->st |= PHP_JIT_CONTEXT_FINISHED;
		}
		
		jit_function_compile(pfunc->func);
	}

	if (nargs != pfunc->sig->nparams) {
		/* throw, function signature does not match */
		efree(jargs);
		efree(args);
		return;
	}

	if (args) {
		zend_uint narg = 0;

		if (zend_get_parameters_array(ht, nargs, args) != SUCCESS) {
			/* throw failed to fetch arguments */
			efree(jargs);
			efree(args);
			return;
		}

		zend_llist_init(&stack, sizeof(void**), php_jit_invoke_stack_dtor, 0);

		while (narg < nargs) {
			if (Z_TYPE_P(args[narg]) == IS_ARRAY) {
				if (!pfunc->sig->params[narg]->pt) {
					/* throw, this is not a pointer type */
					zend_llist_destroy(&stack);
					efree(jargs);
					efree(args);
					return;
				}

				jargs[narg] = php_jit_array_args
						(pfunc, &stack, args[narg], narg TSRMLS_CC);
			} else switch (pfunc->sig->params[narg]->id) {
				case PHP_JIT_TYPE_UINT:
				case PHP_JIT_TYPE_INT:
				case PHP_JIT_TYPE_ULONG:
				case PHP_JIT_TYPE_LONG: {
					jargs[narg] = &Z_LVAL_P(args[narg]);
				} break;

				case PHP_JIT_TYPE_DOUBLE:   jargs[narg] = &Z_LVAL_P(args[narg]);   break;
				case PHP_JIT_TYPE_STRING:   jargs[narg] = &Z_STRVAL_P(args[narg]); break;
				case PHP_JIT_TYPE_VOID_PTR: jargs[narg] = &args[narg]->value;      break;
			}

			narg++;
		}
	}
	
	jit_function_apply(pfunc->func, jargs, &result);

	switch (pfunc->sig->returns->id) {
		case PHP_JIT_TYPE_STRING: ZVAL_STRING(return_value, (char*) result, 1); break;
		
		case PHP_JIT_TYPE_INT:
		case PHP_JIT_TYPE_UINT:
		case PHP_JIT_TYPE_ULONG:
		case PHP_JIT_TYPE_LONG:   ZVAL_LONG(return_value, (long) result); break;
		
		case PHP_JIT_TYPE_DOUBLE: {
			double doubled =
				*(double *) &result;
			ZVAL_DOUBLE(return_value, doubled);
		} break;

		case PHP_JIT_TYPE_VOID_PTR: ZVAL_LONG(return_value, (long) result); break;
		
		default: {
			/* throw type unknown to zend */
		}
	}

	zend_llist_destroy(&stack);
	efree(args);
	efree(jargs);
}

PHP_METHOD(Func, dump) {
	zval *zname = NULL, *zoutput = NULL;
	php_jit_function_t *pfunc;
	php_stream *pstream = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &zname, &zoutput) != SUCCESS) {
		return;
	}

	if (!zname || Z_TYPE_P(zname) != IS_STRING) {
		/* throw expected name string */
		return;
	}

	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	if (!zoutput) {
		jit_dump_function(stdout, pfunc->func, Z_STRVAL_P(zname));
		return;
	}
	
	php_stream_from_zval(pstream, &zoutput);
	
	if (php_stream_can_cast(pstream, PHP_STREAM_AS_STDIO|PHP_STREAM_CAST_TRY_HARD) == SUCCESS) {
		FILE *stdio;
		if (php_stream_cast(pstream, PHP_STREAM_AS_STDIO, (void**)&stdio, 0) == SUCCESS) {
			jit_dump_function(stdio, pfunc->func, Z_STRVAL_P(zname));
		}
	}
}

ZEND_BEGIN_ARG_INFO_EX( php_jit_function_get_param_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, parameter)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_construct_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, context)
	ZEND_ARG_INFO(0, signature)
	ZEND_ARG_INFO(0, parent)
ZEND_END_ARG_INFO()

zend_function_entry php_jit_function_methods[] = {
	PHP_ME(Func, __construct,   php_jit_function_construct_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, compile,       php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, isCompiled,    php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, isNested,      php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, getParent,     php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, getContext,    php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, getSignature,  php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, dump,          php_jit_no_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, __invoke,      NULL,    ZEND_ACC_PUBLIC)
	PHP_FE_END
};
#endif
#endif
