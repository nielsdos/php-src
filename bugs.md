I will be using the new aliased names for the DOM classes in this document.

## DOM\Node class (and its subclasses)

### Properties

* $nodeValue (https://dom.spec.whatwg.org/#dom-node-nodevalue)
  * Decodes and substitutes entities, undoing any kind of entity encoding. Can result in security issues.
  * Is only supposed to return a string for attributes and CharacterData subtypes,
    it should return NULL for other node types. However, it can return a string too for elements for example.
* $nodeName (https://dom.spec.whatwg.org/#dom-node-nodename)
  * For elements: should be the uppercased qualified name for HTML.
* $textContent (https://dom.spec.whatwg.org/#dom-node-textcontent)
  * Returns the empty string instead of NULL for nodes that don't have text content.
  * Should only return text content for DocumentFragment/Element/Attr/CharacterData subtypes.
* $prefix (https://dom.spec.whatwg.org/#concept-element-namespace-prefix)
  * Should be NULL instead of the empty string when unspecified.
  * Is writable but shouldn't be, causes all sorts of weird issues where shared namespace data is changed.

### Methods

* isDefaultNamespace(string $namespace) (https://dom.spec.whatwg.org/#dom-node-isdefaultnamespace)
  * Strictly speaking this should have argument type `?string` instead of `string`.
    The NULL value is treated the same as the empty string.
  * Ignores implicit existence of xml and xmlns namespace.
  * Ignores xmlns attributes, i.e. only works with internal namespace declarations.
* lookupNamespaceURI(?string $prefix) (https://dom.spec.whatwg.org/#dom-node-lookupnamespaceuri)
  * Ignores implicit existence of xml and xmlns namespace.
  * Ignores xmlns attributes, i.e. only works with internal namespace declarations.
* lookupPrefix(string $namespace) (https://dom.spec.whatwg.org/#dom-node-lookupprefix)
  * Strictly speaking this should have argument type `?string` instead of `string`.
    The NULL value is treated the same as the empty string.
  * Ignores xmlns attributes, i.e. only works with internal namespace declarations.
* replaceChild(DOMNode $node, DOMNode $child) (https://dom.spec.whatwg.org/#dom-node-replacechild)
  * Does not fully check the node types of the parent or the child. Some edge cases are not checked.
* appendChild(DOMNode $node) (https://dom.spec.whatwg.org/#dom-node-appendchild)
  * Violates pre-insertion validity. E.g. should not work with attributes.
  * Breaks when appending dtd nodes.
* insertBefore(DOMNode $node, ?DOMNode $child = null) (https://dom.spec.whatwg.org/#dom-node-insertbefore)
  * Violates pre-insertion validity.
* normalize() (https://dom.spec.whatwg.org/#dom-node-normalize)
  * Makes nodes inaccessible if text is merged and the node still has a userland reference.
    This can be fixed unconditionally in the master branch.
  * The behaviour for merge order is different. This is because when all the implementations were
    unified into a single spec, they decided to change the behaviour of the merge order: https://www.w3.org/Bugs/Public/show_bug.cgi?id=19837.
* cloneNode() (https://dom.spec.whatwg.org/#concept-node-clone)
  * Merges adjacent text nodes.

## DOM\Attr class

### Properties

* $name (https://dom.spec.whatwg.org/#dom-attr-name)
  * Should be the qualified name instead of the local name.

## DOM\Text class

### Methods

* splitText(int $offset) (https://dom.spec.whatwg.org/#dom-text-splittext)
  * Should throw a DOMException when $offset is greater than the text's $length.

## DOM\ChildNode and DOM\ParentNode interface

For all the methods in this interface, the pre-insertion validity checking is incomplete.
Source: https://dom.spec.whatwg.org/#concept-node-ensure-pre-insertion-validity

* Step 4 is missing: Should throw a hierarchy request DOMException when the node to insert isn't a DocumentFragment, DocumentType, Element, or CharacterData.
* Step 5 (first part) is missing: Text nodes may not be inserted if the parent is a document, should result in a hierarchy request DOMException.
* Step 5 (second part) is missing: Doctype nodes may only be inserted if the parent is a document, otherwise should result in a hierarchy request DOMException.
* Missing all the validation of step 6.
* Merges adjacent text nodes while it shouldn't.
* Handles the special case of passing a single node incorrectly with regards to error handling.

## DOM\Document class

### Properties
  * $documentURI is supposed to be a URI, but for local files it doesn't prefix the path with "file://".
  * $strictErrorChecking property should only exist on the legacy DOMDocument class, exceptions instead of warnings are the default in the modern-day DOM spec.

### Methods

* createAttribute(string $localName) (https://dom.spec.whatwg.org/#dom-document-createattribute)
  * Should check whether the document is an HTML document. In that case it should lowercase the $localName.
* createAttributeNS(?string $namespace, string $qualifiedName) (https://dom.spec.whatwg.org/#dom-document-createattributens)
  * Fails with a warning if there is no root element in the document. This is due to an implementation detail.
  * Fails when there are other internal namespace declarations with a different prefix.
* createCDATASection(string $data) (https://dom.spec.whatwg.org/#dom-document-createcdatasection)
  * Should throw a NotSupported DOMException if the document is an HTML document.
  * Should throw an InvalidCharacterError DOMException if $data contains "]]>".
* createProcessingInstruction(string $target, string $data = "") (https://dom.spec.whatwg.org/#dom-document-createprocessinginstruction)
  * Should throw an InvalidCharacterError DOMException if $data contains "?>".
* createElement(string $localName, string $value = "") (https://dom.spec.whatwg.org/#dom-document-createelement)
  * Should set $localName to lowercase ASCII if this is an HTML document.
  * Should automatically set the HTML namespace if this is an HTML document.
  * Decodes and substitutes entities, undoing any kind of entity encoding. Can result in security issues.
* createElementNS(?string $namespace, string $qualifiedName, string $value = "") (https://dom.spec.whatwg.org/#internal-createelementns-steps)
  * The validate and extract step is completely broken, allowing bogus stuff like `createElementNS("http://www.w3.org/2000/xmlns/", "svg")`.
  * Decodes and substitutes entities, undoing any kind of entity encoding. Can result in security issues.
* getElementsByTagName(string $qualifiedName) (https://dom.spec.whatwg.org/#concept-getelementsbytagname)
  * Does not take into account casing rules for the HTML namespace vs other namespaces.
* getElementsByTagNameNS(?string $namespace, string $localName) (https://dom.spec.whatwg.org/#concept-getelementsbytagnamens)
  * Due to current implementation issues w.r.t. HTML namespaces, the empty namespace matching can sometimes match elements and sometimes not.
* importNode(DOMNode $node, bool $deep = false) (https://dom.spec.whatwg.org/#dom-document-importnode)
  * Document types cannot be imported.
  * Always warns for unsupported node types, regardless of $strictErrorChecking, instead of warning or throwing a DOMException.
    In spec compliant mode this should always throw in such cases.

## DOM\Element class

### Properties

* $tagName (https://dom.spec.whatwg.org/#dom-element-tagname)
  * Should be the uppercased qualified name for HTML.

### Methods

* getAttributeNode(string $qualifiedName) (https://dom.spec.whatwg.org/#dom-element-getattributenode)
  * Should lowercase the qualified name when working with HTML namespace in an HTML document.
  * Should return NULL instead of false when the attribute doesn't exist.
  * Doesn't correctly match the qualified name of an attribute.
  * Can return a DOMNameSpaceNode instead of an actual attribute (see general issues).
* getAttributeNodeNS(?string $namespace, string $localName) (https://dom.spec.whatwg.org/#dom-element-getattributenodens)
  * As a side note: this method _does_ return NULL instead of false, unlike getAttributeNode!
  * Can return a DOMNameSpaceNode instead of an actual attribute (see general issues).
  * Treats the empty string and NULL $namespace different, while they are actually the same thing.
* getAttribute(string $qualifiedName) (https://dom.spec.whatwg.org/#dom-element-getattribute)
  * Should lowercase the qualified name when working with HTML namespace in an HTML document.
  * Should return NULL instead of the empty string when the attribute doesn't exist because it would be impossible
    to differentiate between a non-existent attribute and an empty attribute otherwise.
  * Internally works with DOMNameSpaceNode (see general issues).
* getAttributeNS(?string $namespace, string $localName) (https://dom.spec.whatwg.org/#dom-element-getattributens)
  * Should return NULL instead of the empty string instead when the attribute doesn't exist.
  * Internally works with DOMNameSpaceNode (see general issues).
  * Treats the empty string and NULL $namespace different, while they are actually the same thing.
* hasAttribute(string $qualifiedName) (https://dom.spec.whatwg.org/#dom-element-hasattribute)
  * Should lowercase the qualified name when working with HTML namespace in an HTML document.
  * Doesn't correctly match the qualified name of an attribute.
  * Internally works with DOMNameSpaceNode (see general issues).
* hasAttributeNS(?string $namespace, string $qualifiedName) (https://dom.spec.whatwg.org/#dom-element-hasattributens)
  * Internally works with DOMNameSpaceNode (see general issues).
  * Treats the empty string and NULL $namespace different, while they are actually the same thing.
* removeAttribute(string $qualifiedName) (https://dom.spec.whatwg.org/#dom-element-removeattribute)
  * Should lowercase the qualified name when working with HTML namespace in an HTML document.
  * Internally works with DOMNameSpaceNode (see general issues).
  * Shouldn't return anything according to spec, but it returns a bool indicating failure or success.
* removeAttributeNS(?string $namespace, string $localName) (https://dom.spec.whatwg.org/#dom-element-removeattributens)
  * Internally works with DOMNameSpaceNode (see general issues).
  * Shouldn't return anything according to spec, but it returns a bool indicating failure or success.
  * Treats the empty string and NULL $namespace different, while they are actually the same thing.
* setAttribute(string $qualifiedName, string $value) (https://dom.spec.whatwg.org/#dom-element-setattribute)
  * Internally works with DOMNameSpaceNode (see general issues).
  * Should lowercase the qualified name when working with HTML namespace in an HTML document.
* setAttributeNS(?string $namespace, string $qualifiedName, string $value) (https://dom.spec.whatwg.org/#dom-element-setattributens)
  * Internally works with DOMNameSpaceNode (see general issues).
  * Violates the namespace well-formedness constraints (see spec step "validate and extract").
  * Treats the empty string and NULL $namespace different, while they are actually the same thing.
* setAttributeNode(DOMAttr $attr) (https://dom.spec.whatwg.org/#dom-element-setattributenode)
  * Behaves incorrectly when namespaced attributes are provided. To solve this, this is nowadays an alias for
    setAttributeNode.
* setAttributeNodeNS(DOMAttr $attr) (https://dom.spec.whatwg.org/#dom-element-setattributenodens)
  * Should't throw WRONG_DOCUMENT_ERR.
  * Should throw INUSE_ATTRIBUTE_ERR when the attribute is already attached to another element.
* insertAdjacent{Text,Element} (https://dom.spec.whatwg.org/#dom-element-insertadjacenttext and https://dom.spec.whatwg.org/#dom-element-insertadjacentelement)
  * Due to other implementation issues of internal methods, these can violate the hierarchy constraints.

## DOM\NamedNodeMap class

Has the same bugs as DOM\Element::getAttribute.

## Issues related to CharacterData-like classes

According to spec, the methods that operate on strings expect unsigned arguments instead of signed arguments.
This allows you to do things like: `$text->substringData(1, -1)` to get the string inside `$text` excluding the first character.
This currently isn't the case and will become possible by this proposal.

## General issues

* The HTML namespace is practically non-existent and not respected.
* Namespace serialization is incorrect when xmlns attributes exist, or when the namespace of an element is the empty namespace (in some cases).
* There is a DOMNameSpaceNode class where namespace declarations are sometimes treated as attributes and sometimes are not. This causes all sorts of inconsistencies. In modern-day DOM spec the internal namespace information is not exposed, but when you see an xmlns declaration they are attributes and can be manipulated properly like attributes. This also causes issues when there is internal namespace information _and_ an xmlns attribute. The DOMNameSpaceNode class is also lacking in features because they try to be like attributes but are not due to implementation problems.
  In modern-day DOM spec, the internal namespace declarations are not attributes.
  Explicit xmlns attributes exist that are just attributes and have no influence on the namespace declaration, they are only there to help serialization.
* Namespace reconciliation can shift nodes between namespaces, which is incorrect.
* The ID attribute is not always respected because the current DOM implementation still has the setAttributeId legacy behaviour. (e.g. setAttribute("id") is a problem).
* Inconsistencies regarding "length 0" vs "\0" handling...
* getElementById checks for xml:id instead of id in an xml document (perhaps not a problem?).
* The XML serialization is incorrect in some cases (related to namespace prefix conflicts and the empty namespace).

## Class hierarchy

The class hierarchy w.r.t. textual nodes is supposed to be:
* CharacterData extends Node (Actually an interface)
  * Text extends CharacterData
    * CDATASection extends Text
  * ProcessingInstruction extends CharacterData
  * Comment extends CharacterData

However in the current implementation, the ProcessingInstruction class extends Node instead of CharacterData.
Also CharacterData is a class instead of an interface in the current implementation.

Not really a major issue, but if we're going to list them all anyway...

## Typing

* As listed above, there are a lot of places where the implementation uses "string" but should actually use "?string".
* DOMNameSpaceNode will never be possibly returned in the spec compliant implementation, so that return type becomes useless.
* There are a lot of return types of the form "T|false" because the current implementation can return false on error
  instead of throwing an exception if "strictErrorChecking" is false. This is a legacy DOM feature that is no longer
  supported in the modern-day DOM spec. For new classes, the return type would become "T" instead of "T|false".
* null vs false return type.

## Bug reports

Both bugsnet and GitHub contain bug reports that are consequences of spec compliance issues.
By implementing this proposal, the following reports will be closed as fixed:

- https://bugs.php.net/bug.php?id=47847
- https://bugs.php.net/bug.php?id=55294
- https://bugs.php.net/bug.php?id=75624
- https://bugs.php.net/bug.php?id=75779
- https://bugs.php.net/bug.php?id=81468
- https://bugs.php.net/bug.php?id=81682
- https://github.com/php/php-src/issues/12850
- https://github.com/php/php-src/issues/11404
- https://github.com/php/php-src/issues/8388

Although this list looks small, the impact of this proposal is huge.
It will fix a lot of issues that are not in this list.
