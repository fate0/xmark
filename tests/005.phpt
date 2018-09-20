--TEST--
Check xrename_class
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

class Hello {
    public function say() {
        echo "I am hello";
    }
}

xrename_class("Hello", "_Hello");

$a = new _Hello();
$a->say();


?>
--EXPECTF--
I am hello