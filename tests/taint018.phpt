--TEST--
Check SQLite3
--SKIPIF--
<?php if (!extension_loaded("xmark") || !extension_loaded("sqlite3")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_classes=SQLite3:MySQLite3
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

class SQLite3 extends MySQLite3 {
    public function prepare($sql, ...$args) {
        if (xcheck($sql)) {
            echo "prepare-xcheck: ".xcheck($sql)."\n";
        }
        return parent::prepare($sql, ...$args);
    }

    public function query($sql, ...$args) {
        if (xcheck($sql)) {
            echo "query-xcheck: ".xcheck($sql)."\n";
        }
        return parent::query($sql, ...$args);
    }
}

$db = new SQLite3(':memory:');

$sql = "select 1";
xmark($sql);

$db->prepare($sql);
$db->query($sql);

?>
--EXPECT--
prepare-xcheck: 1
query-xcheck: 1