--TEST--
Check header
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=header:my_header
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function header($header, $replace=true, $http_response_code=200) {
    if(xcheck($header)) {
        echo "header:xcheck: ".xcheck($header)."\n";
    }

    // TODO:
    // return my_header($header);
}

$str = "Location: " . str_repeat("xx", 2);

xmark($str);

header($str);

?>
--EXPECT--
header:xcheck: 1

