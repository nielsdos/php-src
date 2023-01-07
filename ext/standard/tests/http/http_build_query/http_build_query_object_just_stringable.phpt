--TEST--
http_build_query() function with object that is just stringable (GH-10229)
--FILE--
<?php
class StringableObject {
    public function __toString() : string {
        return "Stringable";
    }
}

$o = new StringableObject();

var_dump(http_build_query(['hello', $o]));
var_dump(http_build_query($o));
var_dump(http_build_query(['hello', $o], numeric_prefix: 'prefix_'));
var_dump(http_build_query($o, numeric_prefix: 'prefix_'));
?>
--EXPECT--
string(20) "0=hello&1=Stringable"
string(12) "0=Stringable"
string(34) "prefix_0=hello&prefix_1=Stringable"
string(19) "prefix_0=Stringable"
