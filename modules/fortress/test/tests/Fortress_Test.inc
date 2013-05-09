<?php

class Fortress_Test extends Ground_Test_Case {
  function setUp() {
    $this->fixture = new Fortress_Fixture($this->database, $this);
    $this->ground->db->drop_all_tables();
    $this->ground->add_module('Ground_Drupal');
  }

  function test_access_basic() {

    $fortress = new Fortress($this->ground);

    $resource = (object) array(
            'trellis' => 'ninja pig',
    );

    $account = (object) array(
            'uid' => 7,
    );

    $this->assertSame(0, $fortress->access(null, $resource));
    $this->assertSame(0, $fortress->access($account, null));
  }

  function test_get_owner_id() {

    $this->assertSame(null, Fortress::get_owner_id(null));
    $this->assertSame(null, Fortress::get_owner_id(new stdClass()));

    $resource = new stdClass();
    $owner = new stdClass();
    $resource->owner = $owner;
    $owner->id = 7;
    $this->assertEquals(7, Fortress::get_owner_id($resource));

    $resource = new stdClass();
    $owner = new stdClass();
    $resource->author = $owner;
    $owner->id = 8;
    $this->assertEquals(8, Fortress::get_owner_id($resource));

    $resource = new stdClass();
    $owner = new stdClass();
    $resource->author = $owner;
    $owner->uid = 8;
    $this->assertEquals(8, Fortress::get_owner_id($resource));

    $resource = new stdClass();
    $owner = new stdClass();
    $resource->author = 9;
    $this->assertEquals(9, Fortress::get_owner_id($resource));

    $resource = new stdClass();
    $owner = new stdClass();
    $resource->owner = 9;
    $this->assertEquals(9, Fortress::get_owner_id($resource));
  }

  function test_get_resource_trellis() {

    $this->assertSame(null, Fortress::get_resource_trellis(null));
    $this->assertSame(null, Fortress::get_resource_trellis(new stdClass()));

    $resource = new stdClass();
    $trellis = new stdClass();
    $resource->trellis = $trellis;
    $trellis->name = 'ninja pig';
    $this->assertEquals('ninja pig', Fortress::get_resource_trellis($resource));

    $resource = new stdClass();
    $resource->type = 'ninja pig';
    $this->assertEquals('ninja pig', Fortress::get_resource_trellis($resource));

    $resource = new stdClass();
    $resource->trellis = 'ninja pig';
    $this->assertEquals('ninja pig', Fortress::get_resource_trellis($resource));
  }

  function test_user_has_role() {
    $account = new stdClass();
    $account->uid = 7;
    $this->assertFalse(Fortress::user_has_role(null, 'admin'));
    $this->assertFalse(Fortress::user_has_role($account, 'admin'));

    $account->roles = array('admin');
    $this->assertFalse(Fortress::user_has_role($account, 'fisherman'));
    $this->assertTrue(Fortress::user_has_role($account, 'admin'));

    $account->roles = array(array('name' => 'admin'));
    $this->assertTrue(Fortress::user_has_role($account, 'admin'));

    $role = new stdClass();
    $role->name = 'king';
    $account->roles = array($role);
    $this->assertTrue(Fortress::user_has_role($account, 'king'));
  }

  function test_query_integration() {
    $this->fixture->populate_database();
    $request = new Bag('vineyard/warrior');
    $result = $this->ground->vineyard_service($request);
    $this->assertGreaterThan(0, count($result->objects));

    $fortress = new Fortress($this->ground);
    $account = new stdClass();
    $account->uid = 7;
    $fortress->account = $account;
    $result = $this->ground->vineyard_service($request);
    $this->assertEquals(0, count($result->objects));

    $fortress->lay_brick(new Public_Trellis_Brick(array('warrior')));

    $result = $this->ground->vineyard_service($request);
    $this->assertGreaterThan(0, count($result->objects));
  }

  function test_update_integration() {
    $this->fixture->populate_database();
    $fortress = new Fortress($this->ground);
    $account = new stdClass();
    $account->uid = 7;
    $fortress->account = $account;
    $hero = array(
        'name' => 'Orion',
        'trellis' => 'warrior',
    );
    $data = array($hero);

    $result = $this->ground->update_objects($data);
    $this->assertEquals(0, count($result->successes));
    $this->assertEquals(1, count($result->failures));

    $fortress->lay_brick(new Public_Trellis_Brick(array('warrior'), 2));

    $result = $this->ground->update_objects($data);
    $this->assertEquals(1, count($result->successes));
    $this->assertEquals(0, count($result->failures));
  }

}
