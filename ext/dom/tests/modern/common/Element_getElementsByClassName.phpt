--TEST--
Dom\Element::getElementsByClassName()
--EXTENSIONS--
dom
--FILE--
<?php

$dom = Dom\HTMLDocument::createFromString(<<<HTML
<div class="  foo bar ">
    <p class="bar">
        <p class="bar"></p>
    </p>
    <b class="bars"></b>
</div>
HTML, LIBXML_NOERROR);
$collection = $dom->documentElement->getElementsByClassName("bar");

echo "There are {$dom->getElementsByClassName("foo \n bar")->count()} items in the document in total\n";

echo "There are {$dom->getElementsByClassName("")->count()} items that match set \"\" in the document in total\n";

echo "There are {$dom->getElementsByClassName(" ")->count()} items that match set \" \" in the document in total\n";

echo "There are {$collection->count()} items\n";

foreach ($collection as $key => $node) {
    var_dump($key, $node->tagName);
    var_dump($node === $collection->item($key));
}

?>
--EXPECT--
There are 3 items
There are 3 items
int(0)
string(3) "DIV"
bool(false)
int(1)
string(1) "P"
bool(false)
