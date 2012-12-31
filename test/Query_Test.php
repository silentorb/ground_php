<?php

class Query_Test extends Ground_Test_Fixtures {
  function setUp() {
    $this->fixture_populated_database();
  }
 
  function test_select() {
    $query = $this->ground->create_query($this->trellis);
    $result = $query->run();
    $objects = $result->objects;
    $this->assertEquals(1, count($objects));
    $this->assertEquals('Bob', $objects[0]->name);
  }

}
