--TEST--
Check ZEND_INIT_USER_CALL
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
--FILE--
<?php

function init_user_dynamic_call_handler($funcname) {
    var_dump($funcname);
}


xregister_opcode_callback(XMARK_INIT_USER_CALL, 'init_user_dynamic_call_handler');

@call_user_func(array("my_class", "my_method"), "test");

--EXPECTF--
array(2) {
  [0]=>
  string(8) "my_class"
  [1]=>
  string(9) "my_method"
}