--TEST--
Check clear run_time_cache
--SKIPIF--
<?php if (!extension_loaded("xmark")) print "skip"; ?>
--INI--
xmark.enable=1
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php


function add_filter($tag) {
    echo "add_filter $tag\n";
    return true;
}

function add_action($tag) {
    return add_filter($tag);
}


function my_add_filter($tag) {
    echo "my_add_filter $tag\n";
    return true;
}


add_action("hello_tag1");

xrename_function("add_filter", "xmark_add_filter");
xrename_function("my_add_filter", "add_filter");


add_action("hello_tag1");
add_action("hello_tag2");
add_action("hello_tag3");

?>
--EXPECTF--
add_filter hello_tag1
my_add_filter hello_tag1
my_add_filter hello_tag2
my_add_filter hello_tag3