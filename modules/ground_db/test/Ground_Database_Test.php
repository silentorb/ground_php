<?php

class Ground_Database_Test extends PHPUnit_Framework_TestCase {
  function setUp() {    
     global $active_db;
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
    $ground = new Ground('ground_test');
    $this->db->create_table($ground->trellises['vineyard_trellis']);
    $count = count($this->db->get_tables());
    $this->assertEquals(1, $count, "Database has 1 table.");
  }

  function test_create_tables() {
    $ground = new Ground('ground_test');
    $this->db->create_tables($ground->trellises);
    $count = count($this->db->get_tables());
    $this->assertEquals(2, $count, "Database has 2 tables.");
  }

  function test_queries() {
    $ground = new Ground('ground_test');
    $db = $this->db;
    $db->create_tables($ground->trellises);
    $db->query("INSERT INTO vineyard_trellises (id, name) VALUES ('5', 'something'), ('8', 'second')");

    $select_all = 'SELECT * FROM vineyard_trellises ORDER BY id';
    $array = $db->query_array($select_all);
    $this->assertEquals('second', $array[1]['name']);

    $objects = $db->query_objects($select_all);
    $this->assertEquals('second', $objects[1]->name);

    $values = $db->query_values('SELECT name FROM vineyard_trellises ORDER BY id');
    $this->assertEquals('second', $values[1]);

    $value = $db->query_value('SELECT name FROM vineyard_trellises WHERE id = 5');
    $this->assertEquals('something', $value);

    $value = $db->query_value('SELECT name FROM vineyard_trellises WHERE id = 8');
    $this->assertEquals('second', $value);
  }

}