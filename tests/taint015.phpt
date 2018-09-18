--TEST--
Check preg_replace_callback
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=preg_replace_callback:my_preg_replace_callback
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function preg_replace_callback($pattern, $callback, $subject, $limit=-1) {
    if (xcheck($callback)) {
        echo "preg_replace_callback:xcheck: ".xcheck($callback)."\n";
    }

    return my_preg_replace_callback($pattern, $callback, $subject, $limit=-1);
}

function test() {
}

$fname = "test";
xmark($fname);

preg_replace_callback("/xxxx/", $fname, "xxxx");


?>
--EXPECTF--
preg_replace_callback:xcheck: 1