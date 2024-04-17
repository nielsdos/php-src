--TEST--
XMLWriter::openStream() 03 - invalidating stream
--EXTENSIONS--
xmlwriter
--FILE--
<?php

$h = fopen("php://output", "w");

$writer = new XMLWriter;
$writer->openStream($h);
$writer->startElement("root");
fclose($h);
$writer->writeAttribute("align", "left");
$writer->endElement();
var_dump($writer->flush());
unset($writer);

?>
--EXPECT--
int(-1)
