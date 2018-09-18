--TEST--
Check rename internal function
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=phpinfo:myphpinfo, print_r:myprint_r
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function phpinfo() {
    echo "rename phpinfo\n";
    print_r("call print_r");
}


function print_r() {
    echo "rename print_r";
}


phpinfo();
?>
--EXPECTF--
rename phpinfo
rename print_r