--TEST--
Check recalc stack size
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
--FILE--
<?php

function hi() {
    echo "hi";
}

function fake_hi() {
    $c = new SQLite3("/tmp/test.db");
    var_dump($c);
}

xrename_function('hi', 'xmark_hi');
xrename_function('fake_hi', 'hi');

hi();

?>
--EXPECTF--
object(SQLite3)#1 (0) {
}