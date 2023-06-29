--TEST--
Bug #80927 (Removing documentElement after creating attribute node: possible use-after-free)
--EXTENSIONS--
dom
--FILE--
<?php
$dom = new DOMDocument();
$dom->appendChild($dom->createElement("html"));
$el = $dom->createAttributeNS("fake_ns", "test:test");
$dom->removeChild($dom->documentElement);
var_dump($el->namespaceURI);
var_dump($el->prefix);
?>
--EXPECT--
string(7) "fake_ns"
string(4) "test"
