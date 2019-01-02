--TEST--
Check for class internal properties
--SKIPIF--
<?php if (!extension_loaded("xmark") || !extension_loaded("mysqli")) print "skip"; ?>
--INI--
error_reporting=off
xmark.enable=1
xmark.rename_classes=mysqli:prvd_mysqli
--FILE--
<?php

$link = @new prvd_mysqli('localhost','root','123456_correct');
var_dump($link);

?>
--EXPECTF--
object(mysqli)#%d (%d) {
  ["affected_rows"]=>
  %s
  ["client_info"]=>
  %s
  ["client_version"]=>
  int(%d)
  ["connect_errno"]=>
  int(%d)
  ["connect_error"]=>
  string(%d) "%s"
  ["errno"]=>
  %s
  ["error"]=>
  %s
  ["error_list"]=>
  %s
  ["field_count"]=>
  %s
  ["host_info"]=>
  %s
  ["info"]=>
  %s
  ["insert_id"]=>
  %s
  ["server_info"]=>
  %s
  ["server_version"]=>
  %s
  ["stat"]=>
  %s
  ["sqlstate"]=>
  %s
  ["protocol_version"]=>
  %s
  ["thread_id"]=>
  %s
  ["warning_count"]=>
  %s
}