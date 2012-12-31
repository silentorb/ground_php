<?php

class Ground_DB_Test extends PHPUnit_Framework_TestCase {
  function setUp() {
    $this->db = new Ground_Database();
    $this->db->connect('ground_test');
    $this->db->drop_all_tables();
  }

  function test_connection() {
    $this->assertNotNull($this->db->connection);
  }

  function test_drop_all_tables() {
    $count = count($this->db->get_tables());
    $this->assertEquals(0, $count, "Database has no tables.");
  }

  function test_create_table() {
    $ground = new Ground();
    $this->db->create_table($ground->trellises['vineyard_trellis']);
    $count = count($this->db->get_tables());
    $this->assertEquals(1, $count, "Database has 1 table.");
  }

  function test_create_tables() {
    $ground = new Ground();
    $this->db->create_tables($ground->trellises);
    $count = count($this->db->get_tables());
    $this->assertEquals(2, $count, "Database has 2 tables.");
  }

}