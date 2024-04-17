--TEST--
XMLWriter::openStream() 04 - open invalidated stream
--EXTENSIONS--
xmlwriter
--FILE--
<?php

$h = fopen("php://output", "w");
fclose($h);

$writer = new XMLWriter;
try {
    $writer->openStream($h);
} catch (TypeError $e) {
    echo $e->getMessage(), "\n";
}

?>
--EXPECT--
XMLWriter::openStream(): supplied resource is not a valid stream resource
