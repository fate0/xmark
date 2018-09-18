--TEST--
ISSUE #26 (PDO checking doesn't work)
--SKIPIF--
<?php if (!extension_loaded("xmark") || !extension_loaded("pdo_sqlite")) print "skip"; ?>
--INI--
xmark.enable=1
xmark.rename_classes=PDO:my_PDO
include_path={PWD}
auto_prepend_file=base.php
--FILE--
<?php

class PDO extends my_PDO {
    public function prepare($sql, $options=array()) {
        if (xcheck($sql)) {
            echo "prepare-xcheck: ".xcheck($sql)."\n";
        }
        return parent::prepare($sql, $options);
    }

    public function query($sql, ...$args) {
        if (xcheck($sql)) {
            echo "query-xcheck: ".xcheck($sql)."\n";
        }
        return parent::query($sql, ...$args);
    }
}


$db = new PDO("sqlite::memory:");
$sql = "select 1";
xmark($sql);
$stmt = $db->prepare($sql);
$stmt = $db->query($sql);

?>
--EXPECT--
prepare-xcheck: 1
query-xcheck: 1