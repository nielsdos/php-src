--TEST--
Creating too long text-like nodes
--EXTENSIONS--
dom
--INI--
memory_limit=-1
--SKIPIF--
<?php
require __DIR__ . '/memory_skipif.inc';
?>
--FILE--
<?php
$str = str_repeat('X', 2**31 + 10);

try {
    new DOMComment($str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

try {
    new DOMCdataSection($str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

try {
    new DOMText($str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

try {
    new DOMProcessingInstruction($str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

try {
    new DOMProcessingInstruction("x", $str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

$dom = new DOMDocument;
try {
    $dom->createTextNode($str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

try {
    $dom->createComment($str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

try {
    $dom->createCDataSection($str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

try {
    $dom->createProcessingInstruction($str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

try {
    $dom->createProcessingInstruction("x", $str);
} catch (ValueError $e) {
    echo $e->getMessage(), "\n";
}

?>
--EXPECT--
DOMComment::__construct(): Argument #1 ($data) must be less than or equal to 2147483647 bytes long
DOMCdataSection::__construct(): Argument #1 ($data) must be less than or equal to 2147483647 bytes long
DOMText::__construct(): Argument #1 ($data) must be less than or equal to 2147483647 bytes long
DOMProcessingInstruction::__construct(): Argument #1 ($name) must be less than or equal to 2147483647 bytes long
DOMProcessingInstruction::__construct(): Argument #2 ($value) must be less than or equal to 2147483647 bytes long
DOMDocument::createTextNode(): Argument #1 ($data) must be less than or equal to 2147483647 bytes long
DOMDocument::createComment(): Argument #1 ($data) must be less than or equal to 2147483647 bytes long
DOMDocument::createCDATASection(): Argument #1 ($data) must be less than or equal to 2147483647 bytes long
DOMDocument::createProcessingInstruction(): Argument #1 ($target) must be less than or equal to 2147483647 bytes long
DOMDocument::createProcessingInstruction(): Argument #2 ($data) must be less than or equal to 2147483647 bytes long
