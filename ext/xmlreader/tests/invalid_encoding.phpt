--TEST--
Passing an invalid character encoding
--EXTENSIONS--
xmlreader
--FILE--
<?php
$reader = new XMLReader();
try {
    $reader->open(__FILE__, "does not exist");
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

$h = fopen("php://memory", "w+");
try {
    XMLReader::openStream($h, encoding: "does not exist");
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
fclose($h);

try {
    $reader->XML('<?xml version="1.0"?><root/>', "does not exist");
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}
?>
--EXPECT--
XMLReader::open(): Argument #2 ($encoding) must be a valid character encoding
XMLReader::openStream(): Argument #3 ($encoding) must be a valid character encoding
XMLReader::XML(): Argument #2 ($encoding) must be a valid character encoding
