--TEST--
Check Taint with more functions
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=strstr:my_strstr, substr:my_substr, str_replace:my_str_replace, str_ireplace:my_str_ireplace, str_pad:my_str_pad, strtolower:my_strtolower, strtoupper:my_strtoupper
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function strstr($string , ...$args) {
    $result = my_strstr($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function substr($string , ...$args) {
    $result = my_substr($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function str_replace($string , ...$args) {
    $result = my_str_replace($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function str_ireplace($string , ...$args) {
    $result = my_str_ireplace($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function str_pad($string , ...$args) {
    $result = my_str_pad($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function strtolower($string , ...$args) {
    $result = my_strtolower($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function strtoupper($string , ...$args) {
    $result = my_strtoupper($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}

$a = "tainted string";
xmark($a);

$b = strstr($a, "s");
var_dump(xcheck($b));

$b = substr($a, 0, 4);
var_dump(xcheck($b));

$b = str_replace("str,", "btr", $a);
var_dump(xcheck($b));

$b = str_ireplace("str,", "btr", $a);
var_dump(xcheck($b));

$b = str_pad($a, 32);
var_dump(xcheck($b));

$b = str_pad("test", 32, $a);
var_dump(xcheck($b));

$b = strtolower($a);
var_dump(xcheck($b));

$b = strtoupper($a);
var_dump(xcheck($b));


?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(false)
bool(true)
bool(true)