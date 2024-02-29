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
#include "namespace_compat.h"

#define DOM_NS_IS_HTML_MAGIC ((void *) 1)

static void dom_ns_compat_mark_attribute(xmlNodePtr node, xmlNsPtr ns)
{
	xmlChar *name;
	if (ns->prefix) {
		/* Can't overflow in practice because it would require the prefix to occupy almost the entire address space. */
		size_t prefix_len = strlen((char *) ns->prefix);
		size_t len = strlen("xmlns:") + prefix_len;
		name = xmlMalloc(len + 1);
		if (UNEXPECTED(name == NULL)) {
			return;
		}
		memcpy(name, "xmlns:", strlen("xmlns:"));
		memcpy(name + strlen("xmlns:"), ns->prefix, prefix_len + 1 /* including '\0' */);
	} else {
		name = xmlStrdup(BAD_CAST "xmlns");
		if (UNEXPECTED(name == NULL)) {
			return;
		}
	}

	xmlSetNsProp(node, NULL, name, ns->href);
	xmlFree(name);
}

void dom_ns_compat_mark_attribute_list(xmlNodePtr node)
{
	xmlNsPtr ns = node->nsDef;
	while (ns != NULL) {
		dom_ns_compat_mark_attribute(node, ns);
		ns = ns->next;
	}
}

// TODO: extend this to other namespaces too?
bool dom_ns_is_html(const xmlNode *nodep)
{
	ZEND_ASSERT(nodep != NULL);
	xmlNsPtr ns = nodep->ns;
	if (ns != NULL) {
		/* cached for fast checking */
		if (ns->_private == DOM_NS_IS_HTML_MAGIC) {
			return true;
		}
		if (xmlStrEqual(ns->href, BAD_CAST DOM_XHTML_NS_URI)) {
			ns->_private = DOM_NS_IS_HTML_MAGIC;
			return true;
		}
	}
	return false;
}

bool dom_ns_is_html_and_document_is_html(const xmlNode *nodep)
{
	ZEND_ASSERT(nodep != NULL);
	return nodep->doc && nodep->doc->type == XML_HTML_DOCUMENT_NODE && dom_ns_is_html(nodep);
}

static xmlNsPtr dom_ns_create_local_as_is_unchecked(xmlDocPtr doc, xmlNodePtr parent, const char *href, xmlChar *prefix)
{
	xmlNsPtr ns = xmlMalloc(sizeof(xmlNs));
	if (UNEXPECTED(ns == NULL)) {
		return NULL;
	}
	memset(ns, 0, sizeof(xmlNs));
	ns->type = XML_LOCAL_NAMESPACE;
	if (href != NULL) {
		ns->href = xmlStrdup(BAD_CAST href);
		if (UNEXPECTED(ns->href == NULL)) {
			xmlFreeNs(ns);
			return NULL;
		}
	}
	if (prefix != NULL) {
		ns->prefix = xmlStrdup(BAD_CAST prefix);
		if (UNEXPECTED(ns->prefix == NULL)) {
			xmlFreeNs(ns);
			return NULL;
		}
	}

	if (parent != NULL) {
		/* Order doesn't matter for spec-compliant mode, insert at front for fast insertion. */
		ns->next = parent->nsDef;
		parent->nsDef = ns;
	} else {
		php_libxml_set_old_ns(doc, ns);
	}

	return ns;
}

xmlNsPtr dom_ns_create_local_as_is(xmlDocPtr doc, xmlNodePtr parent, const char *href, xmlChar *prefix)
{
	ZEND_ASSERT(doc != NULL);

	if (parent == NULL) {
		xmlNsPtr existing = xmlSearchNs(doc, parent, BAD_CAST prefix);
		if (existing != NULL && xmlStrEqual(existing->href, BAD_CAST href)) {
			return existing;
		}
	}

	return dom_ns_create_local_as_is_unchecked(doc, parent, href, prefix);
}

xmlNsPtr dom_ns_fast_get_html_ns(xmlDocPtr docp)
{
	xmlNsPtr nsptr = NULL;
	xmlNodePtr root = xmlDocGetRootElement(docp);

	if (root != NULL) {
		/* Fast path: if the root element itself is in the HTML namespace, return its namespace.
		 * This is the most common case. */
		if (dom_ns_is_html(root)) {
			return root->ns;
		}

		/* Otherwise, we search all the namespaces that the root declares to avoid creating duplicates. */
		nsptr = xmlSearchNsByHref(docp, root, BAD_CAST DOM_XHTML_NS_URI);
		if (nsptr == NULL) {
			nsptr = dom_ns_create_local_as_is_unchecked(docp, root, DOM_XHTML_NS_URI, NULL);
			if (UNEXPECTED(nsptr == NULL)) {
				/* Only happens on OOM. */
				php_dom_throw_error(INVALID_STATE_ERR, /* strict */ true);
				return NULL;
			}
		}
		nsptr->_private = DOM_NS_IS_HTML_MAGIC;
	} else {
		/* If there is no root we fall back to the old namespace list. */
		if (docp->oldNs != NULL) {
			xmlNsPtr tmp = docp->oldNs;
			do {
				if (tmp->_private == DOM_NS_IS_HTML_MAGIC || xmlStrEqual(tmp->href, BAD_CAST DOM_XHTML_NS_URI)) {
					nsptr = tmp;
					nsptr->_private = DOM_NS_IS_HTML_MAGIC;
					break;
				}
				tmp = tmp->next;
			} while (tmp != NULL);
		}

		/* If there was no old namespace list, or the HTML namespace was not in there, create a new one. */
		if (nsptr == NULL) {
			nsptr = dom_ns_create_local_as_is_unchecked(docp, NULL, DOM_XHTML_NS_URI, NULL);
			if (UNEXPECTED(nsptr == NULL)) {
				/* Only happens on OOM. */
				php_dom_throw_error(INVALID_STATE_ERR, /* strict */ true);
				return NULL;
			}
			nsptr->_private = DOM_NS_IS_HTML_MAGIC;
		}
	}

	return nsptr;
}

#endif  /* HAVE_LIBXML && HAVE_DOM */
