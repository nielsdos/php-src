<?php

$original = new DOMDocument();
$original->loadHTML('<!DOCTYPE html>');

$dt = $original->implementation->createDocumentType('html', '', '');
$original->appendChild($dt);
/*
$doctype = $original->doctype->cloneNode();
var_dump($doctype);

$other = new DOMDocument();
$doctype = $other->importNode($original->doctype);
$other->append($doctype);

echo $original->saveXML();
// Deallocating the original document should not affect the imported node
unset($original);
echo $other->saveXML();

echo "-----------------------------\n";
*/