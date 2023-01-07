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
   | Authors: Sara Golemon <pollita@php.net>                              |
   +----------------------------------------------------------------------+
*/

#include "php_http.h"
#include "php_ini.h"
#include "url.h"

#define URL_DEFAULT_ARG_SEP "&"

static void php_url_encode_scalar(zval *scalar, smart_str *form_str,
	int encoding_type, zend_ulong index_int,
	const char *index_string, size_t index_string_len,
	const char *num_prefix, size_t num_prefix_len,
	const char *key_prefix, size_t key_prefix_len,
	const char *key_suffix, size_t key_suffix_len,
	const char *arg_sep, size_t arg_sep_len)
{
	if (form_str->s) {
		smart_str_appendl(form_str, arg_sep, arg_sep_len);
	}
	/* Simple key=value */
	if (key_prefix) {
		smart_str_appendl(form_str, key_prefix, key_prefix_len);
	}
	if (index_string) {
		zend_string *encoded_key;
		if (encoding_type == PHP_QUERY_RFC3986) {
			encoded_key = php_raw_url_encode(index_string, index_string_len);
		} else {
			encoded_key = php_url_encode(index_string, index_string_len);
		}
		smart_str_append(form_str, encoded_key);
		zend_string_free(encoded_key);
	} else {
		/* Numeric key */
		if (num_prefix) {
			smart_str_appendl(form_str, num_prefix, num_prefix_len);
		}
		smart_str_append_long(form_str, index_int);
	}
	if (key_suffix) {
		smart_str_appendl(form_str, key_suffix, key_suffix_len);
	}
	smart_str_appendc(form_str, '=');

	switch (Z_TYPE_P(scalar)) {
		case IS_STRING: {
			zend_string *encoded_data;
			if (encoding_type == PHP_QUERY_RFC3986) {
				encoded_data = php_raw_url_encode(Z_STRVAL_P(scalar), Z_STRLEN_P(scalar));
			} else {
				encoded_data = php_url_encode(Z_STRVAL_P(scalar), Z_STRLEN_P(scalar));
			}
			smart_str_append(form_str, encoded_data);
			zend_string_free(encoded_data);
			break;
		}
		case IS_LONG:
			smart_str_append_long(form_str, Z_LVAL_P(scalar));
			break;
		case IS_DOUBLE: {
			zend_string *encoded_data;
			zend_string *tmp = zend_double_to_str(Z_DVAL_P(scalar));
			if (encoding_type == PHP_QUERY_RFC3986) {
				encoded_data = php_raw_url_encode(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
			} else {
				encoded_data = php_url_encode(ZSTR_VAL(tmp), ZSTR_LEN(tmp));
			}
			smart_str_append(form_str, encoded_data);
			zend_string_free(tmp);
			zend_string_free(encoded_data);
			break;
		}
		case IS_FALSE:
			smart_str_appendc(form_str, '0');
			break;
		case IS_TRUE:
			smart_str_appendc(form_str, '1');
			break;
		/* All possible types are either handled here or previously */
		EMPTY_SWITCH_DEFAULT_CASE();
	}
}

enum property_status {
	PROPERTY_STATUS_UNDEF,
	PROPERTY_STATUS_DYNAMIC,
	PROPERTY_STATUS_NOT_DYNAMIC,
};

static zend_always_inline zval* compute_is_undef_or_dynamic_property(zval *zdata, enum property_status* status) {
	if (Z_TYPE_P(zdata) == IS_INDIRECT) {
		zdata = Z_INDIRECT_P(zdata);
		if (Z_ISUNDEF_P(zdata)) {
			*status = PROPERTY_STATUS_UNDEF;
		}

		*status = PROPERTY_STATUS_NOT_DYNAMIC;
	} else {
		*status = PROPERTY_STATUS_DYNAMIC;
	}
	
	return zdata;
}

static bool has_public_properties(HashTable *ht, zval *type) {
	zend_string *key = NULL;
	zend_ulong _unused_idx;
	zval *zdata = NULL;
	ZEND_ASSERT(ht);
	ZEND_ASSERT(type);

	ZEND_HASH_FOREACH_KEY_VAL(ht, _unused_idx, key, zdata) {
		enum property_status status;
		zdata = compute_is_undef_or_dynamic_property(zdata, &status);
		if (status == PROPERTY_STATUS_UNDEF) {
			continue;
		}
		bool is_dynamic = status == PROPERTY_STATUS_DYNAMIC;

		if (zend_check_property_access(Z_OBJ_P(type), key, is_dynamic) == SUCCESS) {
			/* property visible in this scope */
			return true;
		}
	} ZEND_HASH_FOREACH_END();

	return false;
}

static bool should_handle_object_as_stringable(zval *zdata, zval *tmp) {
	/* If the data object is stringable and has no properties handle it like a string instead of recursively */
	if (Z_TYPE_P(zdata) == IS_OBJECT &&
		!has_public_properties(HASH_OF(zdata), zdata) &&
		Z_OBJ_HT_P(zdata)->cast_object(Z_OBJ_P(zdata), tmp, IS_STRING) == SUCCESS) {
			return true;
	}
	return false;
}

/* {{{ php_url_encode_hash */
PHPAPI void php_url_encode_hash_ex(HashTable *ht, smart_str *formstr,
				const char *num_prefix, size_t num_prefix_len,
				const char *key_prefix, size_t key_prefix_len,
				const char *key_suffix, size_t key_suffix_len,
			  zval *type, const char *arg_sep, int enc_type)
{
	zend_string *key = NULL;
	char *newprefix, *p;
	const char *prop_name;
	size_t arg_sep_len, newprefix_len, prop_len;
	zend_ulong idx;
	zval *zdata = NULL;
	ZEND_ASSERT(ht);

	if (GC_IS_RECURSIVE(ht)) {
		/* Prevent recursion */
		return;
	}

	if (!arg_sep) {
		arg_sep = INI_STR("arg_separator.output");
		if (!arg_sep || !strlen(arg_sep)) {
			arg_sep = URL_DEFAULT_ARG_SEP;
		}
	}
	arg_sep_len = strlen(arg_sep);

	ZEND_HASH_FOREACH_KEY_VAL(ht, idx, key, zdata) {
		enum property_status status;
		zdata = compute_is_undef_or_dynamic_property(zdata, &status);
		if (status == PROPERTY_STATUS_UNDEF) {
			continue;
		}
		bool is_dynamic = status == PROPERTY_STATUS_DYNAMIC;

		/* handling for private & protected object properties */
		if (key) {
			if (type != NULL && zend_check_property_access(Z_OBJ_P(type), key, is_dynamic) != SUCCESS) {
				/* property not visible in this scope */
				continue;
			}

			if (ZSTR_VAL(key)[0] == '\0' && type != NULL) {
				const char *tmp;
				zend_unmangle_property_name_ex(key, &tmp, &prop_name, &prop_len);
			} else {
				prop_name = ZSTR_VAL(key);
				prop_len = ZSTR_LEN(key);
			}
		} else {
			prop_name = NULL;
			prop_len = 0;
		}

		ZVAL_DEREF(zdata);
		if (Z_TYPE_P(zdata) == IS_ARRAY || Z_TYPE_P(zdata) == IS_OBJECT) {
			zval tmp;
			if (should_handle_object_as_stringable(zdata, &tmp)) {
				php_url_encode_scalar(&tmp, formstr,
					enc_type, idx,
					prop_name, prop_len,
					num_prefix, num_prefix_len,
					key_prefix, key_prefix_len,
					key_suffix, key_suffix_len,
					arg_sep, arg_sep_len);
				zval_ptr_dtor(&tmp);
				continue;
			}

			if (key) {
				zend_string *ekey;
				if (enc_type == PHP_QUERY_RFC3986) {
					ekey = php_raw_url_encode(prop_name, prop_len);
				} else {
					ekey = php_url_encode(prop_name, prop_len);
				}
				newprefix_len = key_suffix_len + ZSTR_LEN(ekey) + key_prefix_len + 3 /* %5B */;
				newprefix = emalloc(newprefix_len + 1);
				p = newprefix;

				if (key_prefix) {
					memcpy(p, key_prefix, key_prefix_len);
					p += key_prefix_len;
				}

				memcpy(p, ZSTR_VAL(ekey), ZSTR_LEN(ekey));
				p += ZSTR_LEN(ekey);
				zend_string_free(ekey);

				if (key_suffix) {
					memcpy(p, key_suffix, key_suffix_len);
					p += key_suffix_len;
				}
				*(p++) = '%';
				*(p++) = '5';
				*(p++) = 'B';
				*p = '\0';
			} else {
				char *ekey;
				size_t ekey_len;
				/* Is an integer key */
				ekey_len = spprintf(&ekey, 0, ZEND_LONG_FMT, idx);
				newprefix_len = key_prefix_len + num_prefix_len + ekey_len + key_suffix_len + 3 /* %5B */;
				newprefix = emalloc(newprefix_len + 1);
				p = newprefix;

				if (key_prefix) {
					memcpy(p, key_prefix, key_prefix_len);
					p += key_prefix_len;
				}

				if (num_prefix) {
					memcpy(p, num_prefix, num_prefix_len);
					p += num_prefix_len;
				}

				memcpy(p, ekey, ekey_len);
				p += ekey_len;
				efree(ekey);

				if (key_suffix) {
					memcpy(p, key_suffix, key_suffix_len);
					p += key_suffix_len;
				}
				*(p++) = '%';
				*(p++) = '5';
				*(p++) = 'B';
				*p = '\0';
			}
			GC_TRY_PROTECT_RECURSION(ht);
			php_url_encode_hash_ex(HASH_OF(zdata), formstr, NULL, 0, newprefix, newprefix_len, "%5D", 3, (Z_TYPE_P(zdata) == IS_OBJECT ? zdata : NULL), arg_sep, enc_type);
			GC_TRY_UNPROTECT_RECURSION(ht);
			efree(newprefix);
		} else if (Z_TYPE_P(zdata) == IS_NULL || Z_TYPE_P(zdata) == IS_RESOURCE) {
			/* Skip these types */
			continue;
		} else {
			php_url_encode_scalar(zdata, formstr,
				enc_type, idx,
				prop_name, prop_len,
				num_prefix, num_prefix_len,
				key_prefix, key_prefix_len,
				key_suffix, key_suffix_len,
				arg_sep, arg_sep_len);
		}
	} ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ Generates a form-encoded query string from an associative array or object. */
PHP_FUNCTION(http_build_query)
{
	zval *formdata;
	char *prefix = NULL, *arg_sep=NULL;
	size_t arg_sep_len = 0, prefix_len = 0;
	smart_str formstr = {0};
	zend_long enc_type = PHP_QUERY_RFC1738;

	ZEND_PARSE_PARAMETERS_START(1, 4)
		Z_PARAM_ARRAY_OR_OBJECT(formdata)
		Z_PARAM_OPTIONAL
		Z_PARAM_STRING(prefix, prefix_len)
		Z_PARAM_STRING_OR_NULL(arg_sep, arg_sep_len)
		Z_PARAM_LONG(enc_type)
	ZEND_PARSE_PARAMETERS_END();

	/* Special case when we get an object that's stringable as input */
	zval tmp;
	if (should_handle_object_as_stringable(formdata, &tmp)) {
		php_url_encode_scalar(&tmp, &formstr,
			enc_type, 0,
			NULL, 0,
			prefix, prefix_len,
			NULL, 0,
			NULL, 0,
			arg_sep, arg_sep_len);
		zval_ptr_dtor(&tmp);
	} else {
		php_url_encode_hash_ex(HASH_OF(formdata), &formstr, prefix, prefix_len, NULL, 0, NULL, 0, (Z_TYPE_P(formdata) == IS_OBJECT ? formdata : NULL), arg_sep, (int)enc_type);
	}

	if (!formstr.s) {
		RETURN_EMPTY_STRING();
	}

	smart_str_0(&formstr);

	RETURN_NEW_STR(formstr.s);
}
/* }}} */
