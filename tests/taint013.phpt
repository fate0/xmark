--TEST--
Check file, file_get_contents
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=file_get_contents:my_file_get_contents, file:my_file
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function file($filename, ...$args) {
    if (xcheck($filename)) {
        echo "file:xcheck: ".xcheck($filename)."\n";
    }

    return my_file($filename, ...$args);
}

function file_get_contents($filename, ...$args) {
    if (xcheck($filename)) {
        echo "file_get_contents:xcheck: ".xcheck($filename)."\n";
    }

    return my_file_get_contents($filename, ...$args);
}


function test() {
	$a = __FILE__;
	xmark($a);
    $str = file($a);
	$str = file_get_contents($a);
}
test();


?>
--EXPECTF--
file:xcheck: 1
file_get_contents:xcheck: 1