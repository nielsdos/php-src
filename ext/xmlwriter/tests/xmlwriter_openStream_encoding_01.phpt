--TEST--
XMLWriter::openStream() with encoding 01 - test UTF-8
--EXTENSIONS--
xmlwriter
--FILE--
<?php

$h = fopen("php://output", "w");

$writer = new XMLWriter;
$writer->openStream($h);
$writer->startDocument(encoding: "UTF-8");
$writer->writeComment('ééé');
unset($writer);

?>
--EXPECT--
<?xml version="1.0" encoding="UTF-8"?>
<!--ééé-->
