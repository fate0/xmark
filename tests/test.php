<?php


// save as test.php

function hi() {
    echo "hi";
}

function fake_hi() {
    $c = new SQLite3("/tmp/test.db");
    var_dump($c);
}

xrename_function('hi', 'xmark_hi');
xrename_function('fake_hi', 'hi');

hi();