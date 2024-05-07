--TEST--
Dom\XMLDocument::createFromString 02
--EXTENSIONS--
dom
--FILE--
<?php

$dom = Dom\XMLDocument::createFromString('<?xml version="1.0"?><container/>');
var_dump($dom->saveXmlFile("php://stdout"));

?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<container/>
int(52)
