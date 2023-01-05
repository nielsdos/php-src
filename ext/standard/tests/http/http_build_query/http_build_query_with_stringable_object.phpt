--TEST--
http_build_query() function with stringable object in array (GH-10229)
--FILE--
<?php
class StringableObject {
    public function __toString() : string {
        return "Stringable";
    }
}

$o = new StringableObject();

var_dump(http_build_query(['hello', $o]));
?>
--EXPECT--
string(20) "0=hello&1=Stringable"
