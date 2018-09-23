--TEST--
Check ZEND_DO_ICALL
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
--FILE--
<?php

function do_icall($call, $params) {
    var_dump($call);
    var_dump($params);
}

xregister_opcode_callback(XMARK_DO_ICALL, 'do_icall');

stripos("test", "t");

--EXPECTF--
string(7) "stripos"
array(2) {
  [0]=>
  string(4) "test"
  [1]=>
  string(1) "t"
}