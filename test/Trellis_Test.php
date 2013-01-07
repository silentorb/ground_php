<?php

class Trellis_Test extends Ground_Test_Fixtures {
  function setUp() {
    $this->ground = new Ground();
    $this->trellis = $this->ground->trellises['vineyard_trellis'];
  }

  function test_get_parent_tree() {
    $this->fixture_load_schemas();
    $tree = $this->ground->trellises['warrior']->get_tree();
    $this->assertEquals(2, count($tree));
  }
  
  function test_property_types() {
    $property = $this->trellis->properties['name'];
    $property_type = $property->get_property_type();
    $this->assertEquals('string', $property_type->name);
    $this->assertSame('', $property_type->default);
    $this->assertSame('', $property->get_default());
  }

  function test_plural() {
    $this->assertEquals('vineyard_trellises', $this->trellis->get_table_name());
  }

  function test_create_object() {
    $object = $this->trellis->create_object();
    $this->assertSame('', $object->name);
    $this->assertSame(null, $object->id);
    $this->assertEquals(count($this->trellis->properties), count(get_object_vars($object)));
  }

}
