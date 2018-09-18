--TEST--
Check unerialize
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=unserialize:my_unserialize
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function unserialize($obj) {
    if(xcheck($obj)) {
        echo "unserialize:xcheck: ".xcheck($obj)."\n";
    }

    return my_unserialize($obj);
}

$str = serialize(array());

xmark($str);

unserialize($str);


?>
--EXPECT--
unserialize:xcheck: 1