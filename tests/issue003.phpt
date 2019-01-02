--TEST--
Check for Reflection::getName
--SKIPIF--
<?php if (!extension_loaded("xmark") || !extension_loaded("sqlite3")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=system:prvd_system
--FILE--
<?php

$ret = new ReflectionFunction('prvd_system');
$name = $ret->getName();
var_dump($name);


?>
--EXPECT--
string(11) "prvd_system"
