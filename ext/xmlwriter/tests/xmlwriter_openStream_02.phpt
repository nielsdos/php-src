--TEST--
XMLWriter::openStream() 02 - repeated opens
--EXTENSIONS--
xmlwriter
--FILE--
<?php

$h = fopen("php://output", "w");

$writer = new XMLWriter;
// Test that repeated open calls properly clean up previous used resources
$writer->openMemory();
$writer->openStream($h);
$writer->openStream($h);
$writer->openStream($h);
$writer->startElement("root");
$writer->writeAttribute("align", "left");
$writer->endElement();
unset($writer);

?>
--EXPECT--
<root align="left"/>
