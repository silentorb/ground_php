<?php

class Ground_Test extends Ground_Test_Case {
  function test_initialization() {
    $this->fixture->load_schemas();
    $this->assertGreaterThan(0, count($this->ground->trellises));
     $this->assertEquals('object', gettype($this->ground->trellises['warrior']->parent));
  }

  function test_property_types() {
    $this->assertTrue(count($this->ground->property_types) > 0, 'Property types were loaded.');

    $parent_type = new stdClass();
    $parent_type->field_type = "INT(4)";

    $types = array();
    $types['parent_type'] = new Property_Type('parent_type', $parent_type, $types);
    $child_type = new stdClass();
    $child_type->parent = 'parent_type';
    $child_type = new Property_Type('parent_type', $child_type, $types);
    $this->assertEquals('INT(4)', $child_type->field_type);

    foreach ($this->ground->property_types as $type) {
      $this->assertEquals(1, preg_match('/^[A-Z_0-9]+\s*(?:\(\d+\))?[\w\s]*$/', $type->field_type), "Field Type '$type->field_type' is valid");
    }
  }

}
