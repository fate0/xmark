--TEST--
Bug #61816 (Segmentation fault)
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

$a = "tainted string" . ".\n";
xmark($a);
$b = array("");
$b[0] .= $a;
var_dump(xcheck($b[0]));
$c = new stdClass();
$c->foo = "this is";
$c->foo .= $b[0];
echo $b[0];  // Segmentation fault
var_dump(xcheck($c->foo));

?>
--EXPECT--
bool(true)
echo_handler:xmark: "tainted string.\n" 1
tainted string.
bool(true)