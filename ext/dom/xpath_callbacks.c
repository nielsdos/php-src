/*
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Christian Stocker <chregu@php.net>                          |
   |          Rob Richards <rrichards@php.net>                            |
   |          Niels Dossche <nielsdos@php.net>                            |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#if defined(HAVE_LIBXML) && defined(HAVE_DOM)

#include "php_dom.h"
#include <libxml/parserInternals.h>

static void xpath_callbacks_entry_dtor(zval *zv)
{
	zend_fcall_info_cache *fcc = Z_PTR_P(zv);
	zend_fcc_dtor(fcc);
	efree(fcc);
}

PHP_DOM_EXPORT void php_dom_xpath_callbacks_ctor(php_dom_xpath_callbacks *registry)
{
    ALLOC_HASHTABLE(registry->functions);
	zend_hash_init(registry->functions, 0, NULL, xpath_callbacks_entry_dtor, false);
}

PHP_DOM_EXPORT void php_dom_xpath_callbacks_clean_node_list(php_dom_xpath_callbacks *registry)
{
	if (registry->node_list) {
		zend_hash_destroy(registry->node_list);
		FREE_HASHTABLE(registry->node_list);
		registry->node_list = NULL;
	}
}

PHP_DOM_EXPORT void php_dom_xpath_callbacks_clean_argument_stack(xmlXPathParserContextPtr ctxt, int num_args)
{
	for (int i = 0; i < num_args; i++) {
		xmlXPathObjectPtr obj = valuePop(ctxt);
		xmlXPathFreeObject(obj);
	}
	/* Push sentinel value */
	valuePush(ctxt, xmlXPathNewString((const xmlChar *) ""));
}

PHP_DOM_EXPORT void php_dom_xpath_callbacks_dtor(php_dom_xpath_callbacks *registry)
{
    if (registry->functions) {
		zend_hash_destroy(registry->functions);
		FREE_HASHTABLE(registry->functions);
	}
	php_dom_xpath_callbacks_clean_node_list(registry);
}

PHP_DOM_EXPORT void php_dom_xpath_callbacks_get_gc(php_dom_xpath_callbacks *registry, zend_get_gc_buffer *gc_buffer)
{
    zval *entry;
    ZEND_HASH_FOREACH_VAL(registry->functions, entry) {
        ZEND_ASSERT(Z_TYPE_P(entry) == IS_PTR);
        zend_get_gc_buffer_add_fcc(gc_buffer, Z_PTR_P(entry));
    } ZEND_HASH_FOREACH_END();
}

PHP_DOM_EXPORT HashTable *php_dom_xpath_callbacks_get_gc_for_whole_object(php_dom_xpath_callbacks *registry, zend_object *object, zval **table, int *n)
{
	if (registry->mode == PHP_DOM_REG_FUNC_MODE_SET) {
		zend_get_gc_buffer *gc_buffer = zend_get_gc_buffer_create();
		php_dom_xpath_callbacks_get_gc(registry, gc_buffer);
		zend_get_gc_buffer_use(gc_buffer, table, n);

		if (object->properties == NULL && object->ce->default_properties_count == 0) {
			return NULL;
		} else {
			return zend_std_get_properties(object);
		}
	} else {
		return zend_std_get_gc(object, table, n);
	}
}

PHP_DOM_EXPORT void php_dom_xpath_callbacks_update_method_handler(php_dom_xpath_callbacks* registry, INTERNAL_FUNCTION_PARAMETERS)
{
	zval *entry, registered_value;
	zend_string *name = NULL;
	HashTable *ht = NULL;

	ZEND_PARSE_PARAMETERS_START(0, 1)
		Z_PARAM_OPTIONAL
		Z_PARAM_ARRAY_HT_OR_STR_OR_NULL(ht, name)
	ZEND_PARSE_PARAMETERS_END();

	if (ht) {
		zend_string *key;
		ZEND_HASH_FOREACH_STR_KEY_VAL(ht, key, entry) {
			zend_fcall_info_cache* fcc = emalloc(sizeof(zend_fcall_info));
			char *error;
			if (!zend_is_callable_ex(entry, NULL, 0, NULL, fcc, &error)) {
				zend_argument_type_error(1, "must be an array with valid callbacks as values, %s", error);
				efree(fcc);
				efree(error);
				RETURN_THROWS();
			}

			zend_fcc_addref(fcc);
			ZVAL_PTR(&registered_value, fcc);

			if (!key) {
				zend_string *str = zval_try_get_string(entry);
				if (str) {
					zend_hash_update(registry->functions, str, &registered_value);
					zend_string_release_ex(str, 0);
				} else {
					zend_fcc_dtor(fcc);
					efree(fcc);
					RETURN_THROWS();
				}
			} else {
				if (ZSTR_LEN(key) == 0) {
					zend_argument_value_error(1, "array key must not be empty");
					zend_fcc_dtor(fcc);
					efree(fcc);
					RETURN_THROWS();
				}
				zend_hash_update(registry->functions, key, &registered_value);
			}
		} ZEND_HASH_FOREACH_END();
		registry->mode = PHP_DOM_REG_FUNC_MODE_SET;
	} else if (name) {
		zend_fcall_info_cache* fcc = emalloc(sizeof(zend_fcall_info));
		char *error;
		zval tmp;
		ZVAL_STR(&tmp, name);
		if (!zend_is_callable_ex(&tmp, NULL, 0, NULL, fcc, &error)) {
			zend_argument_type_error(1, "must be a callable, %s", error);
			efree(fcc);
			efree(error);
			RETURN_THROWS();
		}
		zend_fcc_addref(fcc);
		ZVAL_PTR(&registered_value, fcc);
		zend_hash_update(registry->functions, name, &registered_value);
		registry->mode = PHP_DOM_REG_FUNC_MODE_SET;
	} else {
		registry->mode = PHP_DOM_REG_FUNC_MODE_ALL;
	}
}

PHP_DOM_EXPORT zend_result php_dom_xpath_callbacks_call(php_dom_xpath_callbacks *xpath_callbacks, xmlXPathParserContextPtr ctxt, int num_args, php_dom_xpath_nodeset_evaluation_mode evaluation_mode, dom_object *intern, php_dom_xpath_callbacks_proxy_factory proxy_factory)
{
    zval callback_retval;
	zval *params = NULL;
	zend_result result = FAILURE;

	if (UNEXPECTED(num_args == 0)) {
		zend_throw_error(NULL, "Function name must be passed as the first argument");
		goto cleanup_no_obj;
	}

	uint32_t param_count = num_args - 1;
	if (param_count > 0) {
		params = safe_emalloc(param_count, sizeof(zval), 0);
	}

	/* Reverse order to pop values off ctxt stack */
	for (int i = param_count - 1; i >= 0; i--) {
		xmlXPathObjectPtr obj = valuePop(ctxt);
		ZEND_ASSERT(obj != NULL);
		switch (obj->type) {
			case XPATH_STRING:
				ZVAL_STRING(&params[i],  (char *)obj->stringval);
				break;
			case XPATH_BOOLEAN:
				ZVAL_BOOL(&params[i],  obj->boolval);
				break;
			case XPATH_NUMBER:
				ZVAL_DOUBLE(&params[i], obj->floatval);
				break;
			case XPATH_NODESET:
				if (evaluation_mode == PHP_DOM_XPATH_EVALUATE_NODESET_TO_STRING) {
					char *str = (char *)xmlXPathCastToString(obj);
					ZVAL_STRING(&params[i], str);
					xmlFree(str);
				} else if (evaluation_mode == PHP_DOM_XPATH_EVALUATE_NODESET_TO_NODESET) {
					if (obj->nodesetval && obj->nodesetval->nodeNr > 0) {
						array_init_size(&params[i], obj->nodesetval->nodeNr);
						zend_hash_real_init_packed(Z_ARRVAL_P(&params[i]));
						for (int j = 0; j < obj->nodesetval->nodeNr; j++) {
							xmlNodePtr node = obj->nodesetval->nodeTab[j];
							zval child;
							if (UNEXPECTED(node->type == XML_NAMESPACE_DECL)) {
								xmlNodePtr nsparent = node->_private;
								xmlNsPtr original = (xmlNsPtr) node;

								/* Make sure parent dom object exists, so we can take an extra reference. */
								zval parent_zval; /* don't destroy me, my lifetime is transfered to the fake namespace decl */
								php_dom_create_object(nsparent, &parent_zval, intern);
								dom_object *parent_intern = Z_DOMOBJ_P(&parent_zval);

								php_dom_create_fake_namespace_decl(nsparent, original, &child, parent_intern);
							} else {
								proxy_factory(node, &child, intern, ctxt);
							}
							add_next_index_zval(&params[i], &child);
						}
					} else {
						ZVAL_EMPTY_ARRAY(&params[i]);
					}
				}
				break;
			default:
				ZVAL_STRING(&params[i], (char *)xmlXPathCastToString(obj));
				break;
		}
		xmlXPathFreeObject(obj);
	}

	/* Last element of the stack is the function name */
	xmlXPathObjectPtr obj = valuePop(ctxt);
	if (obj->stringval == NULL) {
		zend_type_error("Handler name must be a string");
		goto cleanup;
	}

	const char *function_name = (const char *) obj->stringval;
	size_t function_name_length = strlen(function_name);

    if (xpath_callbacks->mode == PHP_DOM_REG_FUNC_MODE_ALL) {
		zend_fcall_info fci;
		fci.size = sizeof(fci);
		fci.object = NULL;
		fci.retval = &callback_retval;
		fci.param_count = param_count;
		fci.params = params;
		fci.named_params = NULL;
		ZVAL_STRINGL(&fci.function_name, function_name, function_name_length);

		zend_result result = zend_call_function(&fci, NULL);
		zend_string_release_ex(Z_STR(fci.function_name), false);
		if (UNEXPECTED(result == FAILURE)) {
			goto cleanup;
		}
	} else {
		ZEND_ASSERT(xpath_callbacks->mode == PHP_DOM_REG_FUNC_MODE_SET);

		zval *fcc_wrapper = zend_hash_str_find(xpath_callbacks->functions, function_name, function_name_length);
		if (fcc_wrapper) {
			zend_call_known_fcc(Z_PTR_P(fcc_wrapper), &callback_retval, param_count, params, NULL);
		} else {
			zend_throw_error(NULL, "No callback handler \"%s\" registered", function_name);
			goto cleanup;
		}
	}

	if (Z_TYPE(callback_retval) != IS_UNDEF) {
		if (Z_TYPE(callback_retval) == IS_OBJECT && instanceof_function(Z_OBJCE(callback_retval), dom_node_class_entry)) {
			xmlNode *nodep;
			dom_object *obj;
			if (xpath_callbacks->node_list == NULL) {
				xpath_callbacks->node_list = zend_new_array(0);
			}
			Z_ADDREF_P(&callback_retval);
			zend_hash_next_index_insert(xpath_callbacks->node_list, &callback_retval);
			obj = Z_DOMOBJ_P(&callback_retval);
			nodep = dom_object_get_node(obj);
			valuePush(ctxt, xmlXPathNewNodeSet(nodep));
		} else if (Z_TYPE(callback_retval) == IS_FALSE || Z_TYPE(callback_retval) == IS_TRUE) {
			valuePush(ctxt, xmlXPathNewBoolean(Z_TYPE(callback_retval) == IS_TRUE));
		} else if (Z_TYPE(callback_retval) == IS_OBJECT) {
			zend_type_error("Only objects that are instances of DOMNode can be converted to an XPath expression");
			zval_ptr_dtor(&callback_retval);
			goto cleanup;
		} else {
			zend_string *str = zval_get_string(&callback_retval);
			valuePush(ctxt, xmlXPathNewString((xmlChar *) ZSTR_VAL(str)));
			zend_string_release_ex(str, 0);
		}
		zval_ptr_dtor(&callback_retval);
	}

	result = SUCCESS;

cleanup:
	xmlXPathFreeObject(obj);
cleanup_no_obj:
	if (UNEXPECTED(result != SUCCESS)) {
		/* Push sentinel value */
		valuePush(ctxt, xmlXPathNewString((const xmlChar *) ""));
	}
	if (params) {
		for (int i = 0; i < param_count; i++) {
			zval_ptr_dtor(&params[i]);
		}
		efree(params);
	}

    return result;
}

#endif
