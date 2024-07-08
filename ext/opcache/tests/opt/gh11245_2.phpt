--TEST--
GH-11245: In some specific cases SWITCH with one default statement will cause segfault (TMP variation)
--INI--
opcache.enable=1
opcache.enable_cli=1
opcache.optimization_level=0x7FFFBFFF
opcache.opt_debug_level=0x20000
opcache.preload=
zend_test.observer.enabled=0
--EXTENSIONS--
opcache
--FILE--
<?php
class X {
    // Chosen to test for a memory leak.
    static $prop = "aa";
}
switch (++X::$prop) {
    default:
        if (empty($xx)) {return;}
}
?>
--EXPECTF--
$_main:
     ; (lines=5, args=0, vars=1, tmps=1)
     ; (after optimizer)
     ; %s
0000 PRE_INC_STATIC_PROP string("prop") string("X")
0001 T1 = ISSET_ISEMPTY_CV (empty) CV0($xx)
0002 JMPZ T1 0004
0003 RETURN null
0004 RETURN int(1)
