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
   | Authors: Niels Dossche <nielsdos@php.net>                            |
   +----------------------------------------------------------------------+
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#if defined(HAVE_LIBXML) && defined(HAVE_DOM)
#include "php_dom.h"
#include "dom_properties.h"
#include "html5_serializer.h"

static zend_result dom_inner_html_write_string(void *application_data, const char *buf)
{
	smart_str *output = application_data;
	smart_str_appends(output, buf);
	return SUCCESS;
}

static zend_result dom_inner_html_write_string_len(void *application_data, const char *buf, size_t len)
{
	smart_str *output = application_data;
	smart_str_appendl(output, buf, len);
	return SUCCESS;
}

/* https://w3c.github.io/DOM-Parsing/#the-innerhtml-mixin
 * and https://w3c.github.io/DOM-Parsing/#dfn-fragment-serializing-algorithm */
zend_result dom_element_inner_html_read(dom_object *obj, zval *retval)
{
	DOM_PROP_NODE(const xmlNode *, node, obj);

	/* 1. Let context document be the value of node's node document. */
	const xmlDoc *context_document = node->doc;

	/* 2. If context document is an HTML document, return an HTML serialization of node. */
	if (context_document->type == XML_HTML_DOCUMENT_NODE) {
		smart_str output = {0};
		dom_html5_serialize_context ctx;
		ctx.application_data = &output;
		ctx.write_string = dom_inner_html_write_string;
		ctx.write_string_len = dom_inner_html_write_string_len;
		dom_html5_serialize(&ctx, node);
		ZVAL_STR(retval, smart_str_extract(&output));
	}
	/* 3. Otherwise, context document is an XML document; return an XML serialization of node passing the flag require well-formed. */
	else {
		ZEND_ASSERT(context_document->type == XML_DOCUMENT_NODE);

		/* Note: the innerHTML mixin sets the well-formed flag to true. */
		abort(); // TODO
	}

	return SUCCESS;
}

#endif
