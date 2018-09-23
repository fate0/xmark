--TEST--
Check ZEND_DO_UCALL
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
--FILE--
<?php

function do_ucall($call, $params) {
    var_dump($call);
    var_dump($params);
}


xregister_opcode_callback(XMARK_DO_UCALL, 'do_ucall');


function x() {}

x("xxxx", 123, array('aaa'));

?>
--EXPECTF--
string(1) "x"
array(3) {
  [0]=>
  string(4) "xxxx"
  [1]=>
  int(123)
  [2]=>
  array(1) {
    [0]=>
    string(3) "aaa"
  }
}