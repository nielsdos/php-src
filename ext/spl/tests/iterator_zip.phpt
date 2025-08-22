--TEST--
iterator_zip
--FILE--
<?php

foreach (iterator_zip() as $x) {
    var_dump($x);
}

foreach (iterator_zip([]) as $x) {
    var_dump($x);
}

$a = [1, 2, 3];
$b = [4, 5, 6, 7];

foreach (iterator_zip($a, $b) as [$ai, $bi]) {
    var_dump($ai, $bi);
    echo "\n";
}

function gen($i) {
    echo "in gen\n";
    yield $i;
    yield $i+1;
    yield $i+2;
}

foreach (iterator_zip(gen(0), gen(3), gen(6), ["a","b","c"]) as $val) {
    var_dump($val);
}

$a = ['x' => 1, 'y' => 2, 3 => 3];
function gen_with_key($i) {
    echo "in gen_with_key\n";
    yield 'a' => $i;
    yield 'b' => $i+1;
    yield 3 => $i+2;
}

foreach (iterator_zip(gen_with_key(0), $a) as $key => $val) {
    echo "KEYS: ", implode(', ', $key), "\n";
    echo "VALS: ", implode(', ', $val), "\n";
    unset($key);
}

function &gen_reference() {
    $value = 3;

    while ($value > 0) {
        yield $value;
    }
}

foreach (iterator_zip(gen_reference(), gen_reference()) as $vals) {
    var_dump($vals);
    --$vals[0];
    --$vals[1];
}

?>
--EXPECT--
int(1)
int(4)

int(2)
int(5)

int(3)
int(6)

in gen
in gen
in gen
array(4) {
  [0]=>
  int(0)
  [1]=>
  int(3)
  [2]=>
  int(6)
  [3]=>
  string(1) "a"
}
array(4) {
  [0]=>
  int(1)
  [1]=>
  int(4)
  [2]=>
  int(7)
  [3]=>
  string(1) "b"
}
array(4) {
  [0]=>
  int(2)
  [1]=>
  int(5)
  [2]=>
  int(8)
  [3]=>
  string(1) "c"
}
in gen_with_key
KEYS: a, x
VALS: 0, 1
KEYS: b, y
VALS: 1, 2
KEYS: 3, 3
VALS: 2, 3
array(2) {
  [0]=>
  &int(3)
  [1]=>
  &int(3)
}
array(2) {
  [0]=>
  &int(2)
  [1]=>
  &int(2)
}
array(2) {
  [0]=>
  &int(1)
  [1]=>
  &int(1)
}
