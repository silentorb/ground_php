<?php

class Table_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();
  }

  function test_convert_to_property_type() {
    $this->assertEquals('string', Table::convert_to_property_type('VARCHAR(64)'));
    $this->assertEquals('int', Table::convert_to_property_type('INT (11) '));
  }

  function test_trellis_table_name() {
    $this->fixture->load_schemas();
    foreach ($this->ground->trellises as $trellis) {
      $this->assertNotNull($trellis->get_table_name());
    }

    $this->assertEquals('warriors', $this->ground->trellises['warrior']->get_table_name());
  }

  function test_mapping_table_name() {
    $this->fixture->load_schemas();
    $trellis = $this->ground->trellises['base'];
    $this->assertArrayHasKey('base', $this->ground->tables);
    $this->assertNotNull($trellis->table);
    $this->assertEquals('base_objects', $trellis->table->name);
  }

  function test_mapping_property_name() {
    $this->fixture->load_schemas();
    $this->fixture->prepare_database();
    $properties = Table::load_fields($this->ground->db, 'warriors');
    $trellis = $this->ground->trellises['warrior'];

    // Assert that 'age' was mapped to 'warrior_age'
    $this->assertNotNull($trellis->table);
    $this->assertArrayHasKey('age', $trellis->properties);
    $this->assertEquals('warrior_age', $trellis->properties['age']->get_field_name());
    $this->assertArrayHasKey('warrior_age', $properties);
  }

  function test_mapping_outcome() {
    $this->fixture->populate_database();
    $tables = $this->ground->db->get_tables();
    $this->assertContains('base_objects', $tables);
  }

  function test_get_vineyard_layer() {
    $this->fixture->populate_database();
    $this->trellis = $this->ground->trellises['warrior'];
    $this->table = Table::create_from_trellis($this->trellis, $this->ground);
    $this->table->load_from_database();

    $this->assertEquals(3, count($this->table->properties), 'Table fields.');
    $layer = $this->table->get_vineyard_layer();
    $this->assertEquals(3, count($layer->properties), 'Table properties.');
    $this->assertEquals('string', $layer->properties['race']->type);

    foreach ($layer->properties as $property) {
      $this->assertNotNull($property->type);
    }
  }

}
