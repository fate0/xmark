--TEST--
Check namespace
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.enable_rename=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

namespace Test;

function before_rename($param1, $param2) {
    var_dump($param1);
    var_dump($param2);
}

echo "before rename\n";
before_rename("before", 123);

xrename_function('Test\before_rename', 'Test\after_rename');
echo "after rename\n";
after_rename("after", 321);

class Hello {
    public function say() {
        echo "I am hello";
    }
}

xrename_class("Test\Hello", "Test\_Hello");

$a = new _Hello();
$a->say();

--EXPECTF--
before rename
string(6) "before"
int(123)
after rename
string(5) "after"
int(321)
I am hello