--TEST--
Bug #61163 (Passing and using tainted data in specific way crashes)
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
function test($test)
{
	$data .= $test; // $data doesn't exist yet.
}

test($a);
--EXPECTF--
Notice: Undefined variable: data in %sbug61163.php on line %d
