--TEST--
Check Taint with dim assign contact
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

$a = "tainted string";
xmark($a);
$b = array("this is");
$b[0] .= $a;
var_dump(xcheck($b[0]));

$c = new stdClass();
$c->foo = "this is";
$c->foo .= $a;

var_dump(xcheck($c->foo));

?>
--EXPECT--
bool(true)
bool(true)