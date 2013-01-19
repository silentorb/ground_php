<?php

class Query_Test extends Ground_Test_Fixtures {
  function setUp() {
    $this->fixture_populate_database();
  }
 
  function test_select() {
    $query = $this->ground->create_query($this->ground->trellises['warrior']);
    $result = $query->run_as_service();
    $objects = $result->objects;
    $this->assertEquals(1, count($objects));
    $this->assertEquals('Bob', $objects[0]->name);
    
    $query = $this->ground->create_query($this->ground->trellises['character_item']);
    $result = $query->run_as_service();
    $objects = $result->objects;
    $this->assertEquals(1, $objects[0]->owner->id);
  }

}
