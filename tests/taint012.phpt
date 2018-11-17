--TEST--
Check dirname, basename, pathinfo
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_functions=dirname:x_dirname, basename:x_basename, pathinfo:x_pathinfo
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

function dirname($name) {
    $result = x_dirname($name);

    if (xcheck($name)) {
        xmark($result);
    }

    return $result;
}

function basename($name) {
    $result = x_basename($name);

    if (xcheck($name)) {
        xmark($result);
    }

    return $result;
}

function pathinfo($name, $options=PATHINFO_DIRNAME | PATHINFO_BASENAME | PATHINFO_EXTENSION | PATHINFO_FILENAME) {
    $result = x_pathinfo($name, $options);

    if (xcheck($name)) {
        xmark_var($result);
    }

    return $result;
}

function test() {
	$a = "/home/fate0/playstation/assassin's creed" . chr(ord("/"));
	xmark($a);
	echo "====== dirname =======\n";
    echo dirname($a);
    echo "\n====== basename =======\n";
	echo basename($a);
	echo "\n====== pathinfo1 =======\n";
	echo pathinfo($a, PATHINFO_BASENAME);
	echo "\n====== pathinfo2 =======\n";
	echo pathinfo($a)["dirname"];
	echo "\n====== end =======\n";
}

test();

?>
--EXPECT--
====== dirname =======
echo_handler:xmark: '/home/fate0/playstation' 1
/home/fate0/playstation
====== basename =======
echo_handler:xmark: 'assassin\'s creed' 1
assassin's creed
====== pathinfo1 =======
echo_handler:xmark: 'assassin\'s creed' 1
assassin's creed
====== pathinfo2 =======
echo_handler:xmark: '/home/fate0/playstation' 1
/home/fate0/playstation
====== end =======