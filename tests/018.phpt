--TEST--
Check ZEND_FAST_CONCAT
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
--FILE--
<?php

function concat_handler($param1, $param2) {
    var_dump($param1);
    var_dump($param2);
    return $param1.$param2."tail";
}

xregister_opcode_callback(XMARK_FAST_CONCAT, 'concat_handler');


$a = "test";
$z = "$a hello";

var_dump($z);

--EXPECTF--
string(4) "test"
string(6) " hello"
string(14) "test hellotail"