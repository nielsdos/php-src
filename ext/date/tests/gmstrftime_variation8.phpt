--TEST--
Test gmstrftime() function : usage variation - Passing literal related strings to format argument.
--FILE--
<?php
echo "*** Testing gmstrftime() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$timestamp = gmmktime(8, 8, 8, 8, 8, 2008);
setlocale(LC_ALL, "en_US");
date_default_timezone_set("Asia/Calcutta");

//array of values to iterate over
$inputs = array(
      'A literal % character' => "%%",
);

// loop through each element of the array for timestamp

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( gmstrftime($value) );
      var_dump( gmstrftime($value, $timestamp) );
};

?>
--EXPECTF--
*** Testing gmstrftime() : usage variation ***

--A literal % character--

Deprecated: Function gmstrftime() is deprecated since 8.1, use IntlDateFormatter::format() instead in %s on line %d
string(1) "%"

Deprecated: Function gmstrftime() is deprecated since 8.1, use IntlDateFormatter::format() instead in %s on line %d
string(1) "%"
