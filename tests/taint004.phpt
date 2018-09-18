--TEST--
Check Taint with eval
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

$a = "tainted string" . ".";
xmark($a);

eval('$b = $a;');
die($b);

?>
--EXPECT--
exit_handler:xmark: "tainted string." 1
tainted string.