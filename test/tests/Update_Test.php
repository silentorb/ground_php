<?php

class Update_Test extends Ground_Test_Fixtures {
  function setUp() {
    $this->ground = new Ground('ground_test');
    $this->trellis = $this->ground->trellises['vineyard_trellis'];
    $this->object = $this->trellis->create_object();
  }

  function test_insert() {
    $update = new Update($this->trellis, $this->object, $this->ground);
    $result = $update->run(true);
    $this->assertSame($result->seed, $this->object);
    $this->assertSame(1, $result->seed->id);
  }

  function test_update_object_reference() {
    $this->fixture_populate_database();
    $this->insert_object('character_item', array(
        'name' => 'axe',
        'owner' => $this->ninja_bob,
    ));

    $query = $this->ground->create_query('character_item');
    $result = $query->run_as_service();
    $objects = $result->objects;
    $this->assertEquals(1, $objects[1]->owner->id);
  }

}
