--TEST--
Bug #63100 (array_walk_recursive behaves wrongly when taint enabled)
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

$a = array();
$a[0] = "tainted string<>";
xmark($a[0]);

function xxx(&$item) {
    $item = htmlspecialchars($item);
}

array_walk_recursive($a, "xxx");

echo $a[0];

?>
--EXPECT--
tainted string&lt;&gt;