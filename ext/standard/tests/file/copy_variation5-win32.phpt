--TEST--
Test copy() function: usage variations - destination file names(case sensitive)
--SKIPIF--
<?php
if(substr(PHP_OS, 0, 3) != "WIN")
  die("skip only run on Windows");
?>
--FILE--
<?php
/* Test copy() function: Checking case sensitivity in creation of destination file names
     and the existence and size of destination files
*/

echo "*** Test copy() function: checking case sensitivity in creation of destination file names ***\n";
$file_path = __DIR__;
$src_file_name = $file_path."/copy_variation5.tmp";
$file_handle = fopen($src_file_name, "w");
fwrite( $file_handle, str_repeat("Hello2World...\n", 100) );
fclose($file_handle);

/* array of destination file names */
$dest_files = array(

  /* Checking case sensitiveness */
  "COPY5.tmp",
  "COPY5.TMP",
  "CopY5.TMP"
);

echo "Size of the source file before copy operation => ";
var_dump( filesize($src_file_name) );
clearstatcache();

echo "\n-- Now applying copy() on source file to create copies --";
$count = 1;
foreach($dest_files as $dest_file) {

  echo "\n-- Iteration $count --\n";
  $dest_file_name = $file_path."/$dest_file";

  echo "Copy operation => ";
  var_dump( copy($src_file_name, $dest_file_name) );

  echo "Existence of destination file => ";
  var_dump( file_exists($dest_file_name) );

  echo "Destination file name => ";
  print($dest_file_name);
  echo "\n";

  echo "Size of source file => ";
  var_dump( filesize($src_file_name) );
  clearstatcache();

  echo "Size of destination file => ";
  var_dump( filesize($dest_file_name) );
  clearstatcache();

  $count++;
}


$count = 1;
foreach($dest_files as $dest_file) {
  unlink($file_path."/".$dest_file);
  $count++;
}

echo "*** Done ***\n";
?>
--CLEAN--
<?php
unlink(__DIR__."/copy_variation5.tmp");
?>
--EXPECTF--
*** Test copy() function: checking case sensitivity in creation of destination file names ***
Size of the source file before copy operation => int(1500)

-- Now applying copy() on source file to create copies --
-- Iteration 1 --
Copy operation => bool(true)
Existence of destination file => bool(true)
Destination file name => %s/COPY5.tmp
Size of source file => int(1500)
Size of destination file => int(1500)

-- Iteration 2 --
Copy operation => bool(true)
Existence of destination file => bool(true)
Destination file name => %s/COPY5.TMP
Size of source file => int(1500)
Size of destination file => int(1500)

-- Iteration 3 --
Copy operation => bool(true)
Existence of destination file => bool(true)
Destination file name => %s/CopY5.TMP
Size of source file => int(1500)
Size of destination file => int(1500)

Warning: unlink(%s/COPY5.TMP): No such file or directory in %s on line %d

Warning: unlink(%s/CopY5.TMP): No such file or directory in %s on line %d
*** Done ***
