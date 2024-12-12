--TEST--
GH-10571: Assertion `zval_get_type(&(*(zptr))) == 6 && "Concat should return string"' failed - DIM OP
--FILE--
<?php
class A
{
    public array $prop = ["a"];
}

class B
{
    public function __toString()
    {
        global $a;
        $a = "";
        return "";
    }
}

$a = new A();
$a->prop[0] .= new B();
var_dump($a);
?>
--EXPECT--
string(0) ""
