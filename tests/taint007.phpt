--TEST--
Check Taint with functions
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=sprintf:my_sprintf, vsprintf:my_vsprintf, explode:my_explode, implode:my_implode, join:my_join, trim:my_trim, rtrim:my_rtrim, ltrim:my_ltrim
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php


function sprintf($format, $string , ...$args) {
    $result = my_sprintf($format, $string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function vsprintf($format, $args) {
    $result = my_vsprintf($format, $args);

    if (xcheck($format)) {
        xmark($result);
    } else if (xcheck_var($args)) {
        xmark($result);
    }

    return $result;
}


function explode($delimiter, $string , ...$args) {
    $result = my_explode($delimiter, $string, ...$args);

    if (xcheck($string)) {
        xmark_var($result);
    } else if (xcheck($delimiter)) {
        xmark_var($result);
    }

    return $result;
}


function implode($string , ...$args) {
    $result = my_implode($string, ...$args);
    if (xcheck_var($string)) {
        xmark($result);
    } else if (xcheck_var($args[0])) {
        xmark($result);
    }

    return $result;
}


function join($string , ...$args) {
    $result = my_implode($string, ...$args);
    if (xcheck_var($string)) {
        xmark($result);
    } else if (xcheck_var($args[0])) {
        xmark($result);
    }

    return $result;
}


function trim($string , ...$args) {
    $result = my_trim($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function rtrim($string , ...$args) {
    $result = my_rtrim($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


function ltrim($string , ...$args) {
    $result = my_ltrim($string, ...$args);
    if (xcheck($string)) {
        xmark($result);
    }

    return $result;
}


$a = "tainted string";
xmark($a);

$b = sprintf("%s", $a);
var_dump(xcheck($b));

$b = vsprintf("%s", array($a));
var_dump(xcheck($b));

$b = explode(" ", $a);
var_dump(xcheck($b[0]));

$a = implode(" ", $b);
var_dump(xcheck($a));

$a = join(" ", $b);
var_dump(xcheck($a));

$b = trim($a);
var_dump(xcheck($a));

$b = rtrim($a, "a...Z");
var_dump(xcheck($a));

$b = ltrim($a);
var_dump(xcheck($a));

?>
--EXPECTF--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)