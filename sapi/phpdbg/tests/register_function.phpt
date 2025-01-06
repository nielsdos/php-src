--TEST--
Register a function
--INI--
opcache.enable_cli=0
--PHPDBG--
R non_existent
R test
r
--EXPECTF--
[Successful compilation of %s.php]
prompt> [Breakpoint #0 added at %s.php:7]
prompt> [Breakpoint #1 added at %s.php:12]
prompt> [Breakpoint #1 at %s.php:12, hits: 1]
>00012:     default => 'bar', // breakpoint #1
 00013: };
 00014: 
prompt>
--FILE--
<?php

function test($v1) {
    var_dump($v1);
}

test('hello');

//Function registration is completely broken and has use after frees, and it seems utterly pointless to
// I adds a ref to function added to the hashtable, but then tries to destroy it *after* PHP shutdown has already cleared it
//static void php_phpdbg_destroy_registered(zval *data) /* {{{ */
//{
//	zend_function_dtor(data);
//} /* }}} */

?>
