--TEST--
Check ZEND_DO_FCALL_BY_NAME
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
--FILE--
<?php

function do_fcall_by_name($call, $params) {
    var_dump($call);
    var_dump($params);
}

xregister_opcode_callback(XMARK_DO_FCALL_BY_NAME, 'do_fcall_by_name');


if (1 + 2) {
    function hello() {
        echo "hello";
    }
}


hello("aa", 1, array("x", 2));

--EXPECTF--
string(5) "hello"
array(3) {
  [0]=>
  string(2) "aa"
  [1]=>
  int(1)
  [2]=>
  array(2) {
    [0]=>
    string(1) "x"
    [1]=>
    int(2)
  }
}
hello