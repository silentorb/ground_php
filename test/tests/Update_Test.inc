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

    // Test created and modified
    $created = $objects[1]->created;
    $modified = $objects[1]->modified;
    $this->assertGreaterThan(10000, $created);
    $this->assertGreaterThan(10000, $modified);

    // Test updating existing object.
    $this->cyborg = $this->fixture->insert_object('warrior', array(
        'id' => $this->cyborg->id,
        'name' => 'Cirguit',
        'age' => 2,
        ));

    $objects = $this->ground->create_query('warrior')->run();
    $this->assertEquals(2, count($objects));
    $this->assertEquals(2, $objects[1]->age);
    $this->assertGreaterThanOrEqual($modified, $objects[1]->modified);
    $this->assertEquals($created, $objects[1]->created);

    // Not sure I want to keep this behavior.  Currently an update that ommits existing values
    // results in those values becoming null.
    $this->assertNull($objects[1]->race);
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

  function test_insert_author() {
    global $user;
    $storage = $user->uid;
    $user->uid = 7;
    $this->fixture->load_schemas();
    $this->fixture->prepare_database();
    $this->fixture->insert_object('deed', array(
        'name' => 'something special',
    ));

    $objects = $this->ground->create_query('deed')->run();
    $this->assertEquals(7, $objects[0]->author);
    $user->uid = $storage;
  }

  function test_insert_author2() {
    global $user;
    $storage = $user->uid;
    $this->fixture->load_schemas();
    $this->fixture->prepare_database();
    $this->fixture->insert_object('deed', array(
        'name' => 'something special',
    ));

    $objects = $this->ground->create_query('deed')->run();
    $this->assertSame(0, $objects[0]->author);
  }

  function test_custom_cross_table() {
    $db = $this->ground->db;
    $this->fixture->load_schemas();
    $this->fixture->prepare_database();
    $deed = $this->fixture->insert_object('deed', array(
        'id' => 2,
        'branches' => array(
            array(
                'name' => 'no one',
            )
        )
        ));
    $this->assertEquals('no one', $db->query_value("SELECT name FROM branches WHERE id = '1'"));
    $this->assertNotNull($deed->branches);
    $objects = $this->ground->create_query('deed')->run();
    $this->assertSame('no one', $objects[0]->branches[0]->name);

    $cross = $db->query_objects("SELECT * FROM deedbranch");
    $this->assertEquals(1, count($cross));
  }

//  function test_string_keys() {
//    $this->fixture->load_schemas();
//    $this->fixture->prepare_database();
//    $this->fixture->insert_object('string_test', array(
//        'name' => 'orchard',
//        'fruit' => 'pear',
//    ));
//
//    $objects = $this->ground->create_query('string_test')->run();
//    $this->assertEquals('orchard', $objects[0]->name);
//    $this->assertEquals('pear', $objects[0]->fruit);
//  }
}
