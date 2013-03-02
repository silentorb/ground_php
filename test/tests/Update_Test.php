<?php

class Update_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();
    $this->trellis = $this->ground->trellises['vineyard_trellis'];
  }

  function test_insert() {
    $this->fixture->load_schemas();
    $this->fixture->prepare_database();
    $this->object = $this->trellis->create_object();
    $update = new Update($this->trellis, $this->object, $this->ground);
    $result = $update->run(true);
    $this->assertSame($result->seed, $this->object);
    $this->assertSame(1, $result->seed->id);
  }

  function test_update_object_reference() {
    $this->fixture->populate_database();
    $this->fixture->insert_object('character_item', array(
        'name' => 'axe',
        'owner' => $this->fixture->ninja_bob,
    ));

    $query = $this->ground->create_query('character_item');
    $result = $query->run_as_service();
    $objects = $result->objects;
    $this->assertEquals(1, $objects[1]->owner->id);
  }

}
