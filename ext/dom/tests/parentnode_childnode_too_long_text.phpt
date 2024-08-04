--TEST--
Passing a too long string to ChildNode or ParentNode methods causes an exception
--EXTENSIONS--
dom
--INI--
memory_limit=-1
--SKIPIF--
<?php
require __DIR__ . '/memory_skipif.inc';
?>
--FILE--
<?php
$dom = new DOMDocument;
$element = $dom->appendChild($dom->createElement('root'));
$str = str_repeat('X', 2**31 + 10);
try {
    $element->append('x', $str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
try {
    $element->prepend('x', $str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
try {
    $element->after('x', $str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
try {
    $element->before('x', $str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
try {
    $element->replaceWith('x', $str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
try {
    $element->replaceChildren('x', $str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
var_dump($dom->childNodes->count());
var_dump($element->childNodes->count());
?>
--EXPECT--
DOMElement::append(): Argument #2 must be less than or equal to 2147483647 bytes long
DOMElement::prepend(): Argument #2 must be less than or equal to 2147483647 bytes long
DOMElement::after(): Argument #2 must be less than or equal to 2147483647 bytes long
DOMElement::before(): Argument #2 must be less than or equal to 2147483647 bytes long
DOMElement::replaceWith(): Argument #2 must be less than or equal to 2147483647 bytes long
DOMElement::replaceChildren(): Argument #2 must be less than or equal to 2147483647 bytes long
int(1)
int(0)
