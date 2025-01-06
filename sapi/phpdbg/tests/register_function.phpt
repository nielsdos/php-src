--TEST--
Register a function
--INI--
opcache.enable_cli=0
--PHPDBG--
R non_existent
R test
r
--EXPECTF--
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
