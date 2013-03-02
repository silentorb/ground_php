<?php

class Query_Test extends Ground_Test_Case {

  function setUp() {
    parent::setUp();
    $this->fixture->populate_database();
  }

  function test_select() {
    $query = $this->ground->create_query($this->ground->trellises['warrior']);
    $objects = $query->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals('Bob', $objects[0]->name);

    $query = $this->ground->create_query($this->ground->trellises['character_item']);
    $objects = $query->run();
    $this->assertEquals(1, $objects[0]->owner->id);
  }

  // Originally Vineyard was designed to only support links that had explicit definitions within both
  // Trellises, but it has become increasingly useful when embedding into existing systems not to need
  // to insert the connection definition in the pre-existing table.  Note that these new 'implicit'
  // connections only work in one direction, you still need to define both sides of the connection to
  // access it from either direction.
  function test_one_way_reference() {
    // Warrior has no reciprical connection to achievement.
    $this->fixture->insert_object('achievement', array(
        'name' => 'Slay 10000 Hoarse Radishes',
        'warrior' => $this->fixture->ninja_bob->id,
            ));

    $objects = $this->ground->create_query('achievement')->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals('Bob', $objects[0]->warrior->name);
  }

}
