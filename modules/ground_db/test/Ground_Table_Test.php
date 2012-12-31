<?php

class Ground_Table_Test  extends Ground_Test_Fixtures{
  function setUp() {
  }

  function test_convert_to_property_type() {
    $this->assertEquals('string', Ground_Table::convert_to_property_type('VARCHAR(64)'));
    $this->assertEquals('int', Ground_Table::convert_to_property_type('INT (11) '));
  }

  function test_get_vineyard_layer() {
    $this->fixture_populated_database();
    $this->table = Ground_Table::create_from_trellis($this->trellis, $this->ground);
 
    $this->assertEquals(2, count($this->table->fields), 'Table fields.');
    $layer = $this->table->get_vineyard_layer();
    $this->assertEquals(2, count($layer->properties), 'Table properties.');
    $this->assertEquals('string', $layer->properties['name']->type);
    
    foreach ($layer->properties as $property) {
      $this->assertNotNull($property->type);
    }
  }

}
