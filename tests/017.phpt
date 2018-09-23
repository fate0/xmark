--TEST--
Check ZEND_CONCAT
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

xregister_opcode_callback(XMARK_CONCAT, 'concat_handler');

$a = "test3";
$b = "test4";

$z = "test1".$b."test2".$a;
var_dump($z);


?>
--EXPECTF--
string(5) "test1"
string(5) "test4"
string(14) "test1test4tail"
string(5) "test2"
string(23) "test1test4tailtest2tail"
string(5) "test3"
string(32) "test1test4tailtest2tailtest3tail"