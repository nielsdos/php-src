--TEST--
http_build_query() function with stringable object that throws an exception
--FILE--
<?php
class StringableClass {
    public function __toString(): string {
        throw new Exception("exception message");
    }
}
$o = new StringableClass();

try {
    var_dump(http_build_query($o));
} catch (Exception $exception) {
    var_dump($exception->getMessage());
}
?>
--EXPECT--
string(17) "exception message"
