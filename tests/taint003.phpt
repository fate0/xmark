--TEST--
Check Taint with ternary
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

$b = isset($a)? $a : 0;
echo $b;
echo "\n";

$b .= isset($a)? "xxxx" : 0; //a knew mem leak
var_dump(xcheck($b));
exit($b);

?>
--EXPECT--
echo_handler:xmark: "tainted string" 1
tainted string
bool(true)
exit_handler:xmark: "tainted stringxxxx" 1
tainted stringxxxx