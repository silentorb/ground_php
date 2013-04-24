<?php

class Ground_Database_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();
    $this->fixture->load_schemas();
    $this->db = $this->ground->db;
  }

  function test_connection() {
    $this->assertNotNull($this->db->connection);
  }

  function test_drop_all_tables() {
    $count = count($this->db->get_tables());
    $this->assertEquals(0, $count, "Database has no tables.");
  }

  function test_create_table() {
    $this->db->create_table($this->ground->trellises['vineyard_trellis']);
    $count = count($this->db->get_tables());
    $this->assertEquals(1, $count, "Database has 1 table.");
  }

  function test_create_tables() {
    $this->db->create_tables($this->ground->trellises);
    $count = count($this->db->get_tables());
    $this->assertEquals(9, $count);
  }

  function test_queries() {
    $db = $this->db;
    $db->create_tables($this->ground->trellises);
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