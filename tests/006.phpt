--TEST--
Check do_call
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php


function xmark_do_fcall($call, $params) {
    var_dump($call);
}

function xmark_do_icall($call, $params) {
    var_dump($call);
}

function xmark_do_ucall($call, $params) {
    var_dump($call);
}

function xmark_do_fcall_by_name($call, $params) {
    var_dump($call);
}


xregister_opcode_callback(XMARK_DO_FCALL, 'xmark_do_fcall');
xregister_opcode_callback(XMARK_DO_ICALL, 'xmark_do_icall');
xregister_opcode_callback(XMARK_DO_UCALL, 'xmark_do_ucall');
xregister_opcode_callback(XMARK_DO_FCALL_BY_NAME, 'xmark_do_fcall_by_name');



function hallo(&$test1) {
    echo "now in hallo\n";
}

$d = "hallo";
$s = "xxxxxxaaaadd";
$d($s);



?>
--EXPECTF--
string(25) "xregister_opcode_callback"
string(25) "xregister_opcode_callback"
string(5) "hallo"
now in hallo