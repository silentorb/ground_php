<?php

class Trellis_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();
    $this->fixture->load_schemas();
    $this->trellis = $this->ground->trellises['vineyard_trellis'];
  }

  function test_get_parent_tree() {
    $this->fixture->load_schemas();
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

  function test_clone_property() {
    $a = new Trellis('a', $this->ground);
    $b = new Trellis('b', $this->ground);
    $source = new stdClass();
    $source->type = 'string';
    $a->add_property('prop', $source);
    $a->clone_property('prop', $b);
    $this->assertArrayHasKey('prop', $b->properties);
  }

  function test_override_property() {
    $trellis = new Trellis('a', $this->ground);
    $property = $trellis->add_property('prop', new stdClass());
    $property->override_field('link_class', 'Pretend_Link');
    $this->assertNotNull($trellis->table);
    $this->assertArrayHasKey('prop', $trellis->table->properties);
    $this->assertEquals('Pretend_Link', $trellis->table->properties['prop']->link_class);
  }

}
