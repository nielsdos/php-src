--TEST--
GH-10571: Assertion `zval_get_type(&(*(zptr))) == 6 && "Concat should return string"' failed - OP
--FILE--
<?php
class A
{
    public string $prop = "";
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
$a->prop .= new B();
var_dump($a);
?>
--EXPECT--
string(0) ""
