--TEST--
Bug #71497 (DOMDocument::documentURI incorrect path converting)
--ENV--
BASE={PWD}
--EXTENSIONS--
dom
--FILE--
<?php
$doc = new DOMDocument();
$doc->load($_ENV["BASE"] . "/book.xml");
echo $doc->documentURI, "\n";
var_dump(file_exists($doc->documentURI));
?>
--EXPECT--
