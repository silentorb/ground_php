<?php

class Ground_Test extends Ground_Test_Case {
  function test_initialization() {
    $this->fixture->load_schemas();
    $this->assertGreaterThan(0, count($this->ground->trellises));
    $this->assertEquals('object', gettype($this->ground->trellises['warrior']->parent));
  }
  
  function test_sanitize_string() {
    $this->assertEquals('name', Ground_Database::sanitize_string('name'));
    $this->assertEquals('name_2', Ground_Database::sanitize_string('name_2'));
    $this->assertEquals(3, Ground_Database::sanitize_string('3'));
    $this->assertEquals(2, Ground_Database::sanitize_string(2));
    $this->assertEquals('', Ground_Database::sanitize_string("'"));
    $this->assertEquals('', Ground_Database::sanitize_string('"'));
  }

  function test_strip() {
    $sample = array(
        'id' => 10,
        'name' => 'surreal',
    );

    Ground::strip($sample);
    $this->assertEquals(null, $sample['id']);
    $this->assertEquals('surreal', $sample['name']);

    $sample = new stdClass();
    $sample->height = 20;
    $sample->width = 100;
    $sample->child = array(
        'width' => -420,
        'height' => -400,
        'child' => array(
            'height' => 500,
        )
    );

    Ground::strip($sample, array('height'));
    $this->assertEquals(null, $sample->height);
    $this->assertSame(100, $sample->width);
    $this->assertEquals(null, $sample->child['height']);
    $this->assertSame(-420, $sample->child['width']);
    $this->assertEquals(null, $sample->child['child']['height']);
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
