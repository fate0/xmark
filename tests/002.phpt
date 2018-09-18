--TEST--
Check xmark(), xcheck(), xclear()
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

$a = "hello world";
$b = "你好世界";

xmark($a);

var_dump(xcheck($a));
var_dump(xcheck($b));

xclear($a);
var_dump(xcheck($a));

xmark($b);
var_dump(xcheck($b));

xmark($b);
var_dump(xcheck($b));

$a = "hello";
$hello = "aaaa";
xmark($a);
xmark($hello);

echo $$a;
?>
--EXPECTF--
bool(true)
bool(false)
bool(false)
bool(true)
bool(true)
echo_handler:xmark: "aaaa" 1
aaaa