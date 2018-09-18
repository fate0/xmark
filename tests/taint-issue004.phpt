--TEST--
ISSUE #4 (wrong op fetched)
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function dummy(&$a) {
	extract(array("b" => "ccc"));
	$a = $b;
}

$c = "xxx";
xmark($c);
dummy($c);
var_dump($c);

?>
--EXPECT--
string(3) "ccc"