--TEST--
Check Taint function
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=file_put_contents:my_file_put_contents
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function file_put_contents($filename, $data, ...$args) {
    if (xcheck_var($filename)) {
        echo "file_put_contents:xcheck: filename ".xcheck_var($filename)."\n";
    }

    if (xcheck_var($data)) {
        echo "file_put_contents:xcheck: data ".xcheck_var($data)."\n";
    }

    return my_file_put_contents($filename, $data, ...$args);
}


$a = "tainted string";
xmark($a);

print $a."\n";
$a .= '+';
$sql = "select * from {$a}";

file_put_contents("php://output", $a . "\n");
eval("return '$a';");

?>
--EXPECT--
echo_handler:xmark: 'tainted string\n' 1
tainted string
file_put_contents:xcheck: data 1
tainted string+
include_or_eval_handler:xmark: 'return \'tainted string+\';' 1