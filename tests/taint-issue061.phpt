--TEST--
TAINT ISSUE #061 (PHP 7.2.6 SIGSEGV)
--SKIPIF--
<?php if (!extension_loaded('xmark')) print 'skip'; ?>
--INI--
xmark.enable=1
xmark.rename_functions=substr:my_substr, trim:my_trim, rtrim:my_rtrim, ltrim:my_ltrim, implode:my_implode
--FILE--
<?php

function substr(...$args) {
    return call_user_func_array("my_substr", $args);
}

function trim(...$args) {
    return call_user_func_array("my_trim", $args);
}

function rtrim(...$args) {
    return call_user_func_array("my_rtrim", $args);
}

function ltrim(...$args) {
    return call_user_func_array("my_ltrim", $args);
}

function implode(...$args) {
    return call_user_func_array("my_implode", $args);
}


var_dump(substr('abc', 0, 2));
var_dump(trim('abc', 'c'));
var_dump(rtrim('abc', 'c'));
var_dump(ltrim('abc', 'c'));
var_dump(implode(array('a', 'c'), 'b'));
?>
--EXPECT--
string(2) "ab"
string(2) "ab"
string(2) "ab"
string(3) "abc"
string(3) "abc"