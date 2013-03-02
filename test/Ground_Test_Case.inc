<?php

module_load_include('inc', 'ground_php', 'test/Fixtures');

class Ground_Test_Case extends PHPUnit_Framework_TestCase {

  protected $backupGlobals = FALSE;
  public $database = 'ground_test';

  function setUp() {
    $this->fixture = new Ground_Fixture($this->database, $this);
    $this->ground->db->drop_all_tables();
  }

}
