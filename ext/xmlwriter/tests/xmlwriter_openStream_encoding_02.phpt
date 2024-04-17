--TEST--
XMLWriter::openStream() with encoding 02 - test GBK
--EXTENSIONS--
xmlwriter
--FILE--
<?php

$h = fopen("php://output", "w");

$writer = new XMLWriter;
$writer->openStream($h);
$writer->startDocument(encoding: "GBK");
$writer->writeComment('茅茅茅');
unset($writer);

?>
--EXPECT--
<?xml version="1.0" encoding="GBK"?>
<!--ééé-->
