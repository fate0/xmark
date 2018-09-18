--TEST--
Check function call
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

class TClass {
    public function test() {}
}

function test() {
}

$fname = "test";

xmark($fname);

$fname();
call_user_func($fname);
call_user_func_array($fname, array());

$d = new TClass();
$d->$fname();
call_user_func(array($d, $fname));


?>
--EXPECTF--
init_user_dynamic_call_handler:xcheck: "test" 1
init_user_dynamic_call_handler:xcheck: "test" 1
init_user_dynamic_call_handler:xcheck: "test" 1
init_user_dynamic_call_handler:xcheck: "test" 1
init_user_dynamic_call_handler:xcheck: "test" 1
