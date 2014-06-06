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

#define this_func   (PHP_JIT_FETCH_FUNCTION(getThis()))
#define this_func_j (PHP_JIT_FETCH_FUNCTION_I(getThis()))

zend_object_handlers php_jit_function_handlers;

typedef int         (*php_jit_mem_func_t)    (jit_function_t, jit_value_t, jit_value_t, jit_value_t);
typedef jit_value_t (*php_jit_binary_func_t) (jit_function_t, jit_value_t, jit_value_t);
typedef jit_value_t (*php_jit_unary_func_t)  (jit_function_t, jit_value_t);

/* {{{ */
static inline void php_jit_do_mem_op(php_jit_mem_func_t func, php_jit_function_t *pfunc, zval *zin[3], zval *return_value TSRMLS_DC) {
	jit_value_t in[3];
	int result;
	
	in[0]    = PHP_JIT_FETCH_VALUE_I(zin[0]);
	in[1]    = PHP_JIT_FETCH_VALUE_I(zin[1]);
	in[2]    = PHP_JIT_FETCH_VALUE_I(zin[2]);
	
	result = func
		(pfunc->func, in[0], in[1], in[2]);
	
	ZVAL_LONG(return_value, result);
} /* }}} */

/* {{{ */
static inline void php_jit_do_binary_op(php_jit_binary_func_t func, php_jit_function_t *pfunc, zval *zin[2], zval *return_value TSRMLS_DC) {
	jit_value_t in[2];
	php_jit_value_t *pval;
	
	in[0]    = PHP_JIT_FETCH_VALUE_I(zin[0]);
	in[1]    = PHP_JIT_FETCH_VALUE_I(zin[1]);
	
	object_init_ex(return_value, jit_value_ce);

	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = func(pfunc->func, in[0], in[1]);
} /* }}} */

/* {{{ */
static inline void php_jit_do_unary_op(php_jit_unary_func_t func, php_jit_function_t *pfunc, zval *zin, zval *return_value TSRMLS_DC) {
	jit_value_t in;
	php_jit_value_t *pval;

	in       = PHP_JIT_FETCH_VALUE_I(zin);
	
	object_init_ex(return_value, jit_value_ce);

	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = func(pfunc->func, in);
} /* }}} */

/* {{ */
static inline void php_jit_function_invoke_builder(zval *this_ptr, zval *zbuilder TSRMLS_DC) {
	zval *retval_ptr = NULL, 
		 *tmp_ptr = NULL, 
		 params;
	zval closure;
	zend_fcall_info       fci;
	zend_fcall_info_cache fcc;
	zend_uint             nparam = 0;
	php_jit_function_t    *pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	const zend_function   *function = zend_get_closure_method_def(zbuilder TSRMLS_CC);
	int result = FAILURE;
	
	if (function->common.num_args != pfunc->sig->nparams) {
		/* throw incorrect number of argument accepted by builder function */
		return;
	}
	
	/* bind builder function to current scope and object */
	zend_create_closure(&closure, (zend_function*) function, Z_OBJCE_P(this_ptr), this_ptr TSRMLS_CC);

	if (zend_fcall_info_init(&closure, IS_CALLABLE_CHECK_SILENT, &fci, &fcc, NULL, NULL TSRMLS_CC) != SUCCESS) {
		/* throw failed to initialize closure method */
		zval_dtor(&closure);
		return;
	}
	
	/* build params for builder function */
	array_init(&params);
	
	fci.retval_ptr_ptr = &retval_ptr;
	
	while (nparam < pfunc->sig->nparams) {
		zval *o;
		php_jit_value_t *pval;
		
		MAKE_STD_ZVAL(o);

		object_init_ex(o, jit_value_ce);

		pval = PHP_JIT_FETCH_VALUE(o);

		pval->func = pfunc;
		zend_objects_store_add_ref_by_handle(pval->func->h TSRMLS_CC);

		pval->type = pfunc->sig->params[nparam];
		zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);

		pval->value = jit_value_get_param(pfunc->func, nparam);

		add_next_index_zval(&params, o);

		nparam++;
	}

	/* call builder function */
	zend_fcall_info_args(&fci, &params TSRMLS_CC);

	result = zend_call_function(&fci, &fcc TSRMLS_CC);
	
	if (result == FAILURE) {
		/* throw failed to call builder */
		return;
	}
	
	/* cleanup */
	zend_fcall_info_args_clear(&fci, 1);
	
	if (retval_ptr) {
		zval_ptr_dtor(&retval_ptr);
	}

	zval_dtor(&params);
	zval_dtor(&closure);
} /* }} */

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
	zval *zcontext = NULL,
		 *zsignature = NULL,
		 *zbuilder = NULL,
		 *zparent = NULL;
	
	php_jit_function_t *pfunc;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOz|O", &zcontext, jit_context_ce, &zsignature, jit_signature_ce, &zbuilder, zend_ce_closure, &zparent, jit_function_ce) != SUCCESS) {
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
	
	php_jit_function_invoke_builder
		(getThis(), zbuilder TSRMLS_CC);
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

PHP_METHOD(Func, doWhile) {
	zval *op = NULL;
	php_jit_function_t *pfunc;
	jit_value_t condition, stop, compare;
	jit_label_t label[2] = 
		{jit_label_undefined, jit_label_undefined};
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	int result = FAILURE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of", &op, jit_value_ce, &fci, &fcc) != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	condition = PHP_JIT_FETCH_VALUE_I(op);

	stop      = jit_value_create_nint_constant(pfunc->func, jit_type_sys_long, 0);
	
	jit_insn_label(pfunc->func, &label[0]);
	
	compare = jit_insn_gt(pfunc->func, condition, stop);
	
	jit_insn_branch_if_not(pfunc->func, compare, &label[1]);
	{
		zval *retval_ptr = NULL;
		
		fci.retval_ptr_ptr = &retval_ptr;
		fci.params = NULL;
		fci.param_count = 0;
		
		result = zend_call_function(&fci, &fcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder */
			return;
		}
		
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		
		jit_insn_branch(pfunc->func, &label[0]);
	}
	
	jit_insn_label(pfunc->func, &label[1]);
}

PHP_METHOD(Func, doIf) {
	zval *op = NULL;
	php_jit_function_t *pfunc = NULL;
	jit_value_t temp;
	jit_label_t label = jit_label_undefined;
	zend_fcall_info zpfci, znfci;
	zend_fcall_info_cache zpfcc, znfcc;
	zval *zretnull = NULL;
	int result = FAILURE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of|f", &op, jit_value_ce, &zpfci, &zpfcc, &znfci, &znfcc) != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	jit_insn_branch_if_not(pfunc->func, PHP_JIT_FETCH_VALUE_I(op), &label);
	
	zpfci.retval_ptr_ptr = &zretnull;
	zpfci.params = NULL;
	zpfci.param_count = 0;
	
	result = zend_call_function(&zpfci, &zpfcc TSRMLS_CC);
	
	if (result == FAILURE) {
		/* throw failed to call builder function */
		return;
	}
	
	if (zretnull) {
		zval_ptr_dtor(&zretnull);
	}
	
	jit_insn_label(pfunc->func, &label);
	
	if (ZEND_NUM_ARGS() > 3) {
		znfci.retval_ptr_ptr = &zretnull;
		znfci.params = NULL;
		znfci.param_count = 0;

		result = zend_call_function(&znfci, &znfcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder function */
			return;
		}

		if (zretnull) {
			zval_ptr_dtor(&zretnull);
		}
	}
}

PHP_METHOD(Func, doLabel) {
	php_jit_function_t *pfunc = NULL;
	php_jit_label_t   *plabel = NULL;
	zval *zlabel = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|O", &zlabel, jit_label_ce)) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	if (!zlabel) {
		object_init_ex(return_value, jit_label_ce);
		plabel = PHP_JIT_FETCH_LABEL(return_value);
		plabel->func = pfunc;
		zend_objects_store_add_ref_by_handle(plabel->func->h TSRMLS_CC);
	} else plabel = PHP_JIT_FETCH_LABEL(zlabel);
	
	jit_insn_label(pfunc->func, &plabel->label);
}

PHP_METHOD(Func, doBranch) {
	php_jit_function_t *pfunc  = NULL;
	php_jit_label_t    *plabel = NULL;
	zval *zlabel;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zlabel, jit_label_ce) != SUCCESS) {
		return;
	}
	
	pfunc  = PHP_JIT_FETCH_FUNCTION(getThis());
	plabel = PHP_JIT_FETCH_LABEL(zlabel);
	
	RETURN_LONG(jit_insn_branch(pfunc->func, &plabel->label));
}

PHP_METHOD(Func, doBranchIf) {
	php_jit_function_t *pfunc = NULL;
	php_jit_label_t   *plabel = NULL;
	zval *zin = NULL, *zlabel = NULL;
	php_jit_value_t   *pval   = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|O", &zin, jit_value_ce, &zlabel) != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	if (!zlabel) {
		object_init_ex(return_value, jit_label_ce);
		
		plabel = PHP_JIT_FETCH_LABEL(return_value);
		plabel->func = pfunc;
		zend_objects_store_add_ref_by_handle(plabel->func->h TSRMLS_CC);
	} else plabel = PHP_JIT_FETCH_LABEL(zlabel);
	
	pval = PHP_JIT_FETCH_VALUE(zin);
	
	jit_insn_branch_if(pfunc->func, pval->value, &plabel->label);
}

PHP_METHOD(Func, doBranchIfNot) {
	php_jit_function_t *pfunc = NULL;
	php_jit_label_t   *plabel = NULL;
	zval *zin = NULL, *zlabel = NULL;
	php_jit_value_t   *pval   = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|O", &zin, jit_value_ce, &zlabel) != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	if (!zlabel) {
		object_init_ex(return_value, jit_label_ce);
		
		plabel = PHP_JIT_FETCH_LABEL(return_value);
		plabel->func = pfunc;
		zend_objects_store_add_ref_by_handle(plabel->func->h TSRMLS_CC);
	} else plabel = PHP_JIT_FETCH_LABEL(zlabel);
	
	pval = PHP_JIT_FETCH_VALUE(zin);
	
	jit_insn_branch_if_not(pfunc->func, pval->value, &plabel->label);
}

PHP_METHOD(Func, doIfNot) {
	zval *op = NULL;
	php_jit_function_t *pfunc = NULL;
	jit_value_t temp;
	jit_label_t label = jit_label_undefined;
	zend_fcall_info zpfci, znfci;
	zend_fcall_info_cache zpfcc, znfcc;
	zval *zretnull = NULL;
	int result = FAILURE;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Of|f", &op, jit_value_ce, &zpfci, &zpfcc, &znfci, &znfcc) != SUCCESS) {
		return;
	}
	
	pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	
	jit_insn_branch_if(pfunc->func, PHP_JIT_FETCH_VALUE_I(op), &label);
	
	zpfci.retval_ptr_ptr = &zretnull;
	zpfci.params = NULL;
	zpfci.param_count = 0;
	
	result = zend_call_function(&zpfci, &zpfcc TSRMLS_CC);
	
	if (result == FAILURE) {
		/* throw failed to call builder */
		return;
	}

	if (zretnull) {
		zval_ptr_dtor(&zretnull);
	}
	
	jit_insn_label(pfunc->func, &label);
	
	if (ZEND_NUM_ARGS() > 3) {
		znfci.retval_ptr_ptr = &zretnull;
		znfci.params = NULL;
		znfci.param_count = 0;

		result = zend_call_function(&znfci, &znfcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder */
			return;
		}

		if (zretnull) {
			zval_ptr_dtor(&zretnull);
		}
	}
}

PHP_METHOD(Func, doJumpTable) {
	zval *zvalue = NULL, 
		 **zmember = NULL;
	HashTable *table = NULL;
	HashPosition position;
	php_jit_function_t *pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	jit_label_t *labels = NULL, after = jit_label_undefined;
	zend_uint nlabels = 0;
	zend_uint nlabel = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OH", &zvalue, jit_value_ce, &table) != SUCCESS) {
		return;
	}
	
	nlabels = zend_hash_num_elements(table);
	
	if (!nlabels) {
		return;
	}
	
	labels = (jit_label_t*) ecalloc(nlabels, sizeof(jit_label_t));

	for (nlabel = 0; nlabel < nlabels; nlabel++) {
		labels[nlabel] = jit_label_undefined;
	}
	
	jit_insn_jump_table
		(pfunc->func, PHP_JIT_FETCH_VALUE_I(zvalue), labels, nlabels);
	jit_insn_branch
		(pfunc->func, &after);
	
	nlabel = 0;
	
	for (zend_hash_internal_pointer_reset_ex(table, &position);
		zend_hash_get_current_data_ex(table, (void**)&zmember, &position) == SUCCESS;
		zend_hash_move_forward_ex(table, &position)) {
		zend_fcall_info fci;
		zend_fcall_info_cache fcc;
		zval *retval_ptr = NULL;
		int result = FAILURE;
		
		jit_insn_label(pfunc->func, &labels[nlabel]);
		
		if (zend_fcall_info_init(*zmember, IS_CALLABLE_CHECK_SILENT, &fci, &fcc, NULL, NULL TSRMLS_CC) != SUCCESS) {
			/* throw, not callable ? */
			nlabel++;
			continue;
		}
		
		fci.retval_ptr_ptr = &retval_ptr;
		fci.params = NULL;
		fci.param_count = 0;
		
		result = zend_call_function(&fci, &fcc TSRMLS_CC);
		
		if (result == FAILURE) {
			/* throw failed to call builder */
			return;
		}
		
		if (retval_ptr) {
			zval_ptr_dtor(&retval_ptr);
		}
		
		nlabel++;
	}
	
	jit_insn_label(pfunc->func, &after);
	
	efree(labels);
}

PHP_METHOD(Func, doEq) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_eq, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doNe) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_ne, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doLt) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_lt, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doLe) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_le, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doGt) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_gt, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doGe) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_ge, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doCmpl) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_cmpl, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doCmpg) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_cmpg, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doMul) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_mul, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doMulOvf) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_mul_ovf, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAdd) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_add, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAddOvf) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_add_ovf, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doSub) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_sub, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doSubOvf) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_sub_ovf, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doDiv) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_div, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doPow) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_pow, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doRem) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_rem, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doRemIEEE) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_rem_ieee, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doNeg) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_neg, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAnd) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_and, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doOr) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_or, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doXor) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_xor, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doShl) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_shl, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doShr) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_shr, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doUshr) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_ushr, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doSshr) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_sshr, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doToBool) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_to_bool, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doToNotBool) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_to_not_bool, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAcos) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_acos, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAsin) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_asin, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAtan) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_atan, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAtan2) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_atan2, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doMin) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_min, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doMax) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_binary_op(jit_insn_max, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doCeil) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_ceil, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doCos) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_cos, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doCosh) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_cosh, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doExp) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_exp, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doFloor) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}

	php_jit_do_unary_op(jit_insn_floor, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doLog) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_log, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doLog10) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_log10, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doRint) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_rint, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doRound) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_round, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doSin) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sin, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doSinh) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sinh, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doSqrt) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}

	php_jit_do_unary_op(jit_insn_sqrt, this_func, zin, return_value TSRMLS_CC);	
}

PHP_METHOD(Func, doTan) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_tan, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doTanh) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_tan, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAbs) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_abs, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doSign) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_sign, this_func, zin, return_value TSRMLS_CC);
}


PHP_METHOD(Func, doIsNAN) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_nan, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doIsFinite) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_finite, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doIsInf) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_is_inf, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doAlloca) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_alloca, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doLoad) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_load, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doLoadSmall) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_load_small, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doDup) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_dup, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doStore) {
	zval *zin[2] = {NULL, NULL};
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_LONG(jit_insn_store(
		this_func_j,
		PHP_JIT_FETCH_VALUE_I(zin[0]),
		PHP_JIT_FETCH_VALUE_I(zin[1])));
}

PHP_METHOD(Func, doAddressof) {
	zval *zin = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_unary_op(jit_insn_address_of, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doCheckNull) {
	zval *zin = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_check_null(this_func_j, PHP_JIT_FETCH_VALUE_I(zin)));
}

PHP_METHOD(Func, doMemcpy) {
	zval *zin[3] = {NULL, NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &zin[1], jit_value_ce, &zin[2], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_mem_op(jit_insn_memcpy, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doMemmove) {
	zval *zin[3] = {NULL, NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &zin[1], jit_value_ce, &zin[2], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_mem_op(jit_insn_memmove, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doMemset) {
	zval *zin[3] = {NULL, NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &zin[1], jit_value_ce, &zin[2], jit_value_ce) != SUCCESS) {
		return;
	}
	
	php_jit_do_mem_op(jit_insn_memset, this_func, zin, return_value TSRMLS_CC);
}

PHP_METHOD(Func, doLoadElem) {
	zval *zin[2] = {NULL, NULL};
	php_jit_function_t *pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	php_jit_value_t *pval, *lval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	lval = PHP_JIT_FETCH_VALUE(zin[0]);
	
	if (!lval->type->pt) {
		/* throw not a pointer type */
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_load_elem(
		pfunc->func,
		lval->value, 
		PHP_JIT_FETCH_VALUE_I(zin[1]),
		lval->type->type);
	pval->type = lval->type;
	zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
	pval->func = pfunc;
	zend_objects_store_add_ref_by_handle(pval->func->h TSRMLS_CC);
}

PHP_METHOD(Func, doLoadElemAddress) {
	zval *zin[2] = {NULL, NULL};
	php_jit_function_t *pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	php_jit_value_t *pval, *lval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}

	lval = PHP_JIT_FETCH_VALUE(zin[0]);
	
	if (!lval->type->pt) {
		/* throw not a pointer type */
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_load_elem_address(
		pfunc->func,
		lval->value, 
		PHP_JIT_FETCH_VALUE_I(zin[1]), 
		lval->type->type);
	pval->type = lval->type;
	zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
	pval->func = pfunc;
	zend_objects_store_add_ref_by_handle(pval->func->h TSRMLS_CC);
}

PHP_METHOD(Func, doLoadRelative) {
	zval *zin = NULL;
	php_jit_function_t *pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	long index = 0;
	php_jit_value_t *pval, *lval;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ol", &zin, jit_value_ce, &index) != SUCCESS) {
		return;
	}
	
	lval = PHP_JIT_FETCH_VALUE(zin);
	
	if (!lval->type->pt) {
		/* throw not a pointer type */
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_load_relative(
		pfunc->func,
		lval->value,
		index,
		lval->type->type);
	pval->type = lval->type;
	zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
	pval->func = pfunc;
	zend_objects_store_add_ref_by_handle(pval->func->h TSRMLS_CC);
}

PHP_METHOD(Func, doStoreRelative) {
	zval *zin[2] = {NULL, NULL};
	php_jit_function_t *pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	long index = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &index, &zin[1], jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_LONG(jit_insn_store_relative(
		pfunc->func,
		PHP_JIT_FETCH_VALUE_I(zin[0]), 
		index, 
		PHP_JIT_FETCH_VALUE_I(zin[1])));
}

PHP_METHOD(Func, doConvert) {
	zval *zin[2] = {NULL, NULL};
	php_jit_function_t *pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	php_jit_value_t *pval;
	zend_bool overflow = 0;
	long index = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO|b", &zin[0], jit_value_ce, &zin[1], jit_type_ce, &overflow) != SUCCESS) {
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_convert(
		pfunc->func,
		PHP_JIT_FETCH_VALUE_I(zin[0]),
		PHP_JIT_FETCH_TYPE_I(zin[1]), overflow);
	pval->type = PHP_JIT_FETCH_TYPE(zin[1]);
	zend_objects_store_add_ref_by_handle(pval->type->h TSRMLS_CC);
}

PHP_METHOD(Func, doStoreElem) {
	zval *zin[3] = {NULL, NULL, NULL};
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OOO", &zin[0], jit_value_ce, &zin[1], jit_value_ce, &zin[2], jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_store_elem(
		this_func_j,
		PHP_JIT_FETCH_VALUE_I(zin[0]),
		PHP_JIT_FETCH_VALUE_I(zin[1]), 
		PHP_JIT_FETCH_VALUE_I(zin[2])));
}

PHP_METHOD(Func, doDefaultReturn) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_default_return(this_func_j));
}

PHP_METHOD(Func, doGetCallStack) {
	php_jit_function_t *pfunc = PHP_JIT_FETCH_FUNCTION(getThis());
	php_jit_value_t    *pval = NULL;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	object_init_ex(return_value, jit_value_ce);
	
	pval = PHP_JIT_FETCH_VALUE(return_value);
	pval->value = jit_insn_get_call_stack(pfunc->func);
	pval->func  = pfunc;
	zend_objects_store_add_ref_by_handle(pval->func->h TSRMLS_CC);
}

PHP_METHOD(Func, doReturn) {
	zval *zin = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_return(this_func_j, PHP_JIT_FETCH_VALUE_I(zin)));
}

PHP_METHOD(Func, doPush) {
	zval *zin = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zin, jit_value_ce) != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_push(this_func_j, PHP_JIT_FETCH_VALUE_I(zin)));
}

PHP_METHOD(Func, doPop) {
	zval *zin = NULL;
	long nitems = 1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &nitems) != SUCCESS) {
		return;
	}
	
	RETURN_LONG(jit_insn_pop_stack(this_func_j, (jit_nint) nitems));
}

PHP_METHOD(Func, doDeferPop) {
	zval *zin = NULL;
	long nitems = 1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &nitems) != SUCCESS) {
		return;
	}
	
	RETURN_LONG(jit_insn_defer_pop_stack(this_func_j, (jit_nint) nitems));
}

PHP_METHOD(Func, doFlushDeferPop) {
	zval *zin = NULL;
	long nitems = 1;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &nitems) != SUCCESS) {
		return;
	}
	
	RETURN_LONG(jit_insn_flush_defer_pop(this_func_j, (jit_nint) nitems));
}

PHP_METHOD(Func, doReturnPtr) {
	zval *zin[2] = {NULL, NULL};

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_type_ce) != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_return_ptr
		(this_func_j, PHP_JIT_FETCH_VALUE_I(zin[0]), PHP_JIT_FETCH_TYPE_I(zin[2])));
}

PHP_METHOD(Func, doPushPtr) {
	zval *zin[2] = {NULL, NULL};
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OO", &zin[0], jit_value_ce, &zin[1], jit_type_ce) != SUCCESS) {
		return;
	}
	
	RETURN_BOOL(jit_insn_push_ptr
		(this_func_j, PHP_JIT_FETCH_VALUE_I(zin[0]), PHP_JIT_FETCH_TYPE_I(zin[2])));
}

PHP_METHOD(Func, doCall) {
	zval *zcall = NULL, *zsignature = NULL;
	HashTable *zparams;
	HashPosition position;
	zval **zparam;
	jit_value_t *args;
	zend_uint arg = 0;
	long flags = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "OH|l", &zcall, jit_function_ce, &zparams, &flags) != SUCCESS) {
		return;
	}

	args = (jit_value_t*) ecalloc(zend_hash_num_elements(zparams), sizeof(jit_value_t));

	for (zend_hash_internal_pointer_reset_ex(zparams, &position);
		zend_hash_get_current_data_ex(zparams, (void**)&zparam, &position) == SUCCESS;
		zend_hash_move_forward_ex(zparams, &position)) {
		args[arg] = PHP_JIT_FETCH_VALUE_I(*zparam);
		arg++;
	}

	object_init_ex(return_value, jit_value_ce);

	{
		php_jit_function_t *pfunc = 
			PHP_JIT_FETCH_FUNCTION(getThis());
		php_jit_function_t *pcall = 
			PHP_JIT_FETCH_FUNCTION(zcall);
		php_jit_value_t *pval =
			PHP_JIT_FETCH_VALUE(return_value);
		
		pval->value = jit_insn_call(
			pfunc->func,
			NULL,
			pcall->func,
			pcall->sig->type,
			args, arg, flags);
		pval->func = pfunc;
		zend_objects_store_add_ref_by_handle(pval->func->h TSRMLS_CC);
	}

	efree(args);
}

ZEND_BEGIN_ARG_INFO_EX( php_jit_function_get_param_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, parameter)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_construct_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, context)
	ZEND_ARG_INFO(0, signature)
	ZEND_ARG_INFO(0, builder)
	ZEND_ARG_INFO(0, parent)
ZEND_END_ARG_INFO()


ZEND_BEGIN_ARG_INFO_EX(php_jit_function_ternary_arginfo, 0, 0, 3)
	ZEND_ARG_INFO(0, op1) 
	ZEND_ARG_INFO(0, op2) 
	ZEND_ARG_INFO(0, op3)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_binary_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, op1) 
	ZEND_ARG_INFO(0, op2) 
ZEND_END_ARG_INFO()
	
ZEND_BEGIN_ARG_INFO_EX(php_jit_function_unary_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, op) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doWhile_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, block)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doIncrement_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, op)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doDecrement_arginfo, 0, 0, 1) 
	ZEND_ARG_INFO(0, op)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doLabel_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, label)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doBranch_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, label) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doBranchIf_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, op)
	ZEND_ARG_INFO(0, label) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doBranchIfNot_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, op)
	ZEND_ARG_INFO(0, label) 
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doIf_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, op)
	ZEND_ARG_INFO(0, positive)
	ZEND_ARG_INFO(0, negative)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doCall_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, function)
	ZEND_ARG_INFO(0, signature)
	ZEND_ARG_INFO(0, params)
	ZEND_ARG_INFO(0, flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doMemcpy_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, dest)
	ZEND_ARG_INFO(0, src)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doMemmove_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, dest)
	ZEND_ARG_INFO(0, src)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doMemset_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, dest)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()
	
ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doLoadElem_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, index)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doLoadRelative_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doStoreRelative_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, offset)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doStoreElem_arginfo, 0, 0, 3) 
	ZEND_ARG_INFO(0, base)
	ZEND_ARG_INFO(0, index)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doJumpTable_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, table)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doConvert_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, type)
	ZEND_ARG_INFO(0, overflow)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doReturnPtr_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doPushPtr_arginfo, 0, 0, 2) 
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(php_jit_function_doPop_arginfo, 0, 0, 0) 
	ZEND_ARG_INFO(0, items)
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
	
	/* */
	PHP_ME(Func, doLabel,      php_jit_function_doLabel_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(Func, doBranch,     php_jit_function_doBranch_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doBranchIf,   php_jit_function_doBranchIf_arginfo,    ZEND_ACC_PUBLIC)
	PHP_ME(Func, doBranchIfNot,php_jit_function_doBranchIfNot_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, doIf,         php_jit_function_doIf_arginfo,          ZEND_ACC_PUBLIC)
	PHP_ME(Func, doIfNot,      php_jit_function_doIf_arginfo,          ZEND_ACC_PUBLIC)
	PHP_ME(Func, doWhile,      php_jit_function_doWhile_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(Func, doMul,        php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doMulOvf,     php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAdd,        php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAddOvf,     php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doSub,        php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doSubOvf,     php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doDiv,        php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doPow,        php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doRem,        php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doRemIEEE,    php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doNeg,        php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAnd,        php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doOr,         php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doXor,        php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doShl,        php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doShr,        php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doUshr,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doSshr,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doEq,         php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doNe,         php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLt,         php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLe,         php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doGt,         php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doGe,         php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doCmpl,       php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doCmpg,       php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doToBool,     php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doToNotBool,  php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAcos,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAsin,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAtan,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAtan2,      php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doMin,        php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doMax,        php_jit_function_binary_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doCeil,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doCos,        php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doCosh,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doExp,        php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doFloor,           php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLog,             php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLog10,           php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doRint,            php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doRound,           php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doSin,             php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doSinh,            php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doSqrt,            php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doTan,             php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doTanh,            php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAbs,             php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doSign,            php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doIsNAN,           php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doIsFinite,        php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doIsInf,           php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doCall,            php_jit_function_doCall_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAlloca,          php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLoad,            php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLoadSmall,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doDup,             php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doStore,           php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doAddressof,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doCheckNull,       php_jit_function_unary_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doMemcpy,          php_jit_function_doMemcpy_arginfo,   ZEND_ACC_PUBLIC)
	PHP_ME(Func, doMemmove,         php_jit_function_doMemmove_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(Func, doMemset,          php_jit_function_doMemset_arginfo,   ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLoadElem,        php_jit_function_doLoadElem_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLoadRelative,    php_jit_function_doLoadRelative_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(Func, doLoadElemAddress, php_jit_function_doLoadElem_arginfo,      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doStoreElem,       php_jit_function_doStoreElem_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doStoreRelative,   php_jit_function_doStoreRelative_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(Func, doJumpTable,       php_jit_function_doJumpTable_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doPush,            php_jit_function_unary_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Func, doPushPtr,         php_jit_function_doPushPtr_arginfo,       ZEND_ACC_PUBLIC)
	PHP_ME(Func, doPop,             php_jit_function_doPop_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Func, doDeferPop,        php_jit_function_doPop_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Func, doFlushDeferPop,   php_jit_function_doPop_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Func, doReturn,          php_jit_function_unary_arginfo,           ZEND_ACC_PUBLIC)
	PHP_ME(Func, doReturnPtr,       php_jit_function_doReturnPtr_arginfo,     ZEND_ACC_PUBLIC)
	PHP_ME(Func, doDefaultReturn,   php_jit_no_arginfo,                      ZEND_ACC_PUBLIC)
	PHP_ME(Func, doGetCallStack,    php_jit_no_arginfo,                      ZEND_ACC_PUBLIC)
	/* */
	PHP_FE_END
};
#endif
#endif
