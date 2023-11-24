<?php

$dom = new DOMDocument;
$dom->loadXML(<<<XML
<?xml version="1.0"?>
<html lang="en">
    <head xmlns="http://www.w3.org/1999/xhtml">
        <title property="foo">Hello world</title>
        <script src="/foo.js" />
    </head>
    <body xmlns="http://www.w3.org/1999/xhtml">
        <h1>hello world.</h1>
        <p>test</p>
        <br/>
        <p>test 2</p>
        <default:p xmlns:default="http://www.w3.org/1999/xhtml" class="foo" id="import">namespace prefixed</default:p>
    </body>
</html>
XML);

// Note the HTMLDocument class!
$dom2 = DOM\HTMLDocument::createEmpty();
$imported = $dom2->importNode($dom->documentElement, true);
$dom2->appendChild($imported);

echo $dom2->saveXML();
