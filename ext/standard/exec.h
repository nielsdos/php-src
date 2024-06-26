/*
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | https://www.php.net/license/3_01.txt                                 |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Rasmus Lerdorf <rasmus@lerdorf.on.ca>                        |
   +----------------------------------------------------------------------+
*/

#ifndef EXEC_H
#define EXEC_H

PHP_MINIT_FUNCTION(proc_open);
PHP_MINIT_FUNCTION(exec);

PHPAPI zend_string *php_escape_shell_cmd(const zend_string *unescaped_cmd);
PHPAPI zend_string *php_escape_shell_arg(const zend_string *unescaped_arg);
PHPAPI int php_exec(int type, const char *cmd, zval *array, zval *return_value);

#endif /* EXEC_H */
