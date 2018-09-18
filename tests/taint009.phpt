--TEST--
Fixed bug that tainted info lost if a string is parsed by htmlspecialchars
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php


$a = "tainted string";
xmark($a); //must use concat to make the string not a internal string(introduced in 5.4)

$b = htmlspecialchars($a);
var_dump(xcheck($b));
var_dump(xcheck($a));


?>
--EXPECTF--
bool(false)
bool(true)
