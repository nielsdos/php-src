--TEST--
GH-10571: Assertion `zval_get_type(&(*(zptr))) == 6 && "Concat should return string"' failed - DIM OP resize
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
        // Force a resize, but it will work on a copy instead
        for ($i = 0; $i < 1000; $i++) {
            $a->prop[] = $i;
        }
        return "x";
    }
}

$a = new A();
$a->prop[0] .= new B();
var_dump(count($a->prop));
var_dump($a->prop[0]);
?>
--EXPECT--
int(1001)
string(1) "a"
