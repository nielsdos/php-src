--TEST--
Test http_build_query() function: usage variations - testing Stringable without __toString
--FILE--
<?php
$gmp_object = gmp_init(123);
var_dump(http_build_query($gmp_object));
?>
--EXPECT--
string(5) "0=123"
