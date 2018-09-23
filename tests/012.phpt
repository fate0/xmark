--TEST--
Check ZEND_EXIT
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
--FILE--
<?php

function exit_handler($string) {
    echo "in exit_handler $string\n";
}

xregister_opcode_callback(XMARK_EXIT, 'exit_handler');


exit('quit');

echo "hello?";

?>
--EXPECTF--
in exit_handler quit
quit