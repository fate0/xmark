--TEST--
Check ZEND_ECHO
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
--FILE--
<?php

error_reporting(0);

function echo_handler($string) {
    echo "in echo_handler $string";
}

xregister_opcode_callback(XMARK_ECHO, 'echo_handler');

class A {
    public function say(&$str) {
        echo $str;
    }

    public function __toString() {
        return "A\n";
    }
}

function say(&$str) {
    echo $str;
}

echo "hello\n";
print "hello\n";

$d = "test\n";
echo $d;
say($d);


$a = new A();
$a->say($d);
echo $a;

$f = array(1, 2, 3, 4);
echo $f;

?>
--EXPECTF--
in echo_handler hello
hello
in echo_handler hello
hello
in echo_handler test
test
in echo_handler test
test
in echo_handler test
test
in echo_handler A
A
in echo_handler ArrayArray