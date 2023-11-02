--TEST--
Check xsltprocessor::registerPHPFunctions with string and not allowed function
--EXTENSIONS--
xsl
--FILE--
<?php
include __DIR__ .'/prepare.inc';
$phpfuncxsl = new domDocument();
$phpfuncxsl->load(__DIR__."/phpfunc.xsl");
if(!$phpfuncxsl) {
  echo "Error while parsing the xsl document\n";
  exit;
}
$proc->importStylesheet($phpfuncxsl);
var_dump($proc->registerPHPFunctions('strpos'));
try {
  var_dump($proc->transformToXml($dom));
} catch (Throwable $e) {
  echo $e->getMessage(), "\n";
}
?>
--EXPECT--
NULL
No callback handler "ucwords" registered
