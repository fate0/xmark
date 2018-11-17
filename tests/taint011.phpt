--TEST--
Check assign_ref and global keyword
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function main() {
    global $var;
    $a = "tainted string\n";
    xmark($a);
    $var = $a;
    echo $var;
}

main();
echo $var;

?>
--EXPECT--
echo_handler:xmark: 'tainted string\n' 1
tainted string
echo_handler:xmark: 'tainted string\n' 1
tainted string
