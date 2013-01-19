<?php
print getcwd() . "\n";

chdir(dirname(__FILE__) . '/..');
print getcwd();
$_SERVER['REMOTE_ADDR'] = '127.0.0.1';
require_once './includes/bootstrap.inc';
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
module_load_include('php', 'ground_php', 'test/Ground_Test_Fixtures');
