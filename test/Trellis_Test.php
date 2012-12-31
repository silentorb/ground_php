<?php

class Trellis_Test extends PHPUnit_Framework_TestCase {
  function setUp() {
    $this->ground = new Ground();
    $this->trellis = $this->ground->trellises['vineyard_trellis'];
  }

  function test_property_types() {
    $property = $this->trellis->properties['name'];
    $property_type = $property->get_property_type();
    $this->assertEquals('string', $property_type->name);
    $this->assertSame('', $property_type->default);
    $this->assertSame('', $property->get_default());
  }

  function test_plural() {
    $this->assertEquals('vineyard_trellises', $this->trellis->get_plural());
  }

  function test_create_object() {
    $object = $this->trellis->create_object();
    $this->assertSame('', $object->name);
    $this->assertSame(null, $object->id);
    $this->assertEquals(count($this->trellis->properties), count(get_object_vars($object)));
  }

}
