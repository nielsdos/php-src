--TEST--
Get properties for DOM nodes
--EXTENSIONS--
dom
--FILE--
<?php
$doc = new DOMDocument;
try {
    json_encode($doc);
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}
try {
    (array) $doc;
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}
try {
    var_export($doc);
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}
?>
--EXPECT--
DOMDocument cannot be encoded to JSON because the properties are virtual and do not have a raw value
DOMDocument cannot be cast to an array because the properties are virtual and do not have a raw value
\DOMDocument::__set_state(array(
))DOMDocument cannot be exported because the representation would be insufficient to restore the object from
