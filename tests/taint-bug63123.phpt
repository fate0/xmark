--TEST--
Bug #63123 (Hash pointer should be reset at the end of function:php_taint_mark_strings)
--SKIPIF--
<?php if (!extension_loaded("xmark") || !extension_loaded("sqlite3")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=explode:my_explode
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function explode($delimiter, $string , ...$args) {
    $result = my_explode($delimiter, $string, ...$args);

    if (xcheck($string)) {
        xmark_var($result, xcheck($string));
    } else if (xcheck($delimiter)) {
        xmark_var($result, xcheck($delimiter));
    }

    return $result;
}


$str = "a\n,b\n";
xmark($str);
$a = explode(',', $str);


foreach ($a as $key => $val) {
    echo $val;
}


?>
--EXPECT--
echo_handler:xmark: "a\n" 1
a
echo_handler:xmark: "b\n" 1
b