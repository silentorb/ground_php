<?php

class Update_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();
    $this->fixture->load_schemas();
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

  function test_insert_object_reference() {
    $this->fixture->populate_database();
    $axe = $this->fixture->insert_object('character_item', array(
        'name' => 'axe',
        'owner' => $this->fixture->ninja_bob->id,
            ));

    $this->assertEquals(2, $axe->id);

    $objects = $this->ground->create_query('character_item')->run();
    $this->assertEquals(1, $objects[1]->owner->id);

    // Insert Embedded Object
    $this->cyborg = $this->fixture->insert_object('warrior', array(
        'name' => 'Cirguit',
        'race' => 'cyborg',
        'age' => 1,
        'inventory' => array(
            array(
                'name' => 'laser',
            )
        )
            ));

    $objects = $this->ground->create_query('character_item')->run();
    $this->assertEquals('laser', $objects[2]->name);
    $this->assertEquals(2, $objects[2]->owner->id);

    $objects = $this->ground->create_query('warrior')->run();
    $this->assertGreaterThan(1, count($objects));
    $this->assertEquals('object', gettype($objects[1]->inventory[0]));
  }

  function test_insert_new_embedded_reference() {
    $this->fixture->load_schemas();
    $this->fixture->prepare_database();

    // Insert Embedded Object
    $this->cyborg = $this->fixture->insert_object('character_item', array(
        'name' => 'laser',
        'owner' => array(
            'name' => 'Cirguit',
            'race' => 'cyborg',
            'age' => 1,
        )
            ));

    $objects = $this->ground->create_query('warrior')->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals(1, $objects[0]->inventory[0]->id);

    $objects = $this->ground->create_query('character_item')->run();
    $this->assertEquals(1, $objects[0]->owner->id);
  }
}
