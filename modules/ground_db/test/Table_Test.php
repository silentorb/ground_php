<?php

class Table_Test  extends Ground_Test_Fixtures{
  function setUp() {
  }

  function test_convert_to_property_type() {
    $this->assertEquals('string', Table::convert_to_property_type('VARCHAR(64)'));
    $this->assertEquals('int', Table::convert_to_property_type('INT (11) '));
  }
  
  function test_trellis_table_name() {
    $this->fixture_load_schemas();
    foreach($this->ground->trellises as  $trellis) {
      $this->assertNotNull($trellis->get_table_name());
    }
    
    $this->assertEquals('warriors', $this->ground->trellises['warrior']->get_table_name());
  }

  function test_get_vineyard_layer() {
    $this->fixture_populate_database();
    $this->table = Table::create_from_trellis($this->trellis, $this->ground);
 
    $this->assertEquals(3, count($this->table->fields), 'Table fields.');
    $layer = $this->table->get_vineyard_layer();
    $this->assertEquals(3, count($layer->properties), 'Table properties.');
    $this->assertEquals('string', $layer->properties['race']->type);
    
    foreach ($layer->properties as $property) {
      $this->assertNotNull($property->type);
    }
  }

}
