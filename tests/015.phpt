--TEST--
Check ZEND_INIT_DYNAMIC_CALL
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


xregister_opcode_callback(XMARK_INIT_DYNAMIC_CALL, 'init_user_dynamic_call_handler');

function my_function() {}

$function = "my_function";
$function();

--EXPECTF--
string(11) "my_function"