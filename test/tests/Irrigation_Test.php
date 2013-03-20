<?php

class Temp {
  function nothing() {
    $this->called = true;
  }

}

class Irrigation_Test extends Ground_Test_Case {
  function test_get_path_array() {
    $result = Irrigation::get_path_array('home/item/action', 'home');
    $this->assertEquals($result[0], 'item');
    $this->assertEquals($result[1], 'action');

    $result = Irrigation::get_path_array('/home/item/action', 'home');
    $this->assertEquals($result[0], 'item');
    $this->assertEquals($result[1], 'action');

    $result = Irrigation::get_path_array('home/item/action', '/home');
    $this->assertEquals($result[0], 'item');
    $this->assertEquals($result[1], 'action');

    $result = Irrigation::get_path_array('item/action', '/');
    $this->assertEquals($result[0], 'item');
    $this->assertEquals($result[1], 'action');

    $result = Irrigation::get_path_array('item/12', '/');
    $this->assertEquals($result[0], 'item');
    $this->assertEquals($result[1], 12);
  }

  function test_find_channel() {
    $this->fixture->load_schemas();
    $irrigation = new Irrigation($this->ground);
    $irrigation->add_channel('@trellis/resurrect', $this, 'resurrect');
    $irrigation->add_channel('harvest', $this, 'harvest_food');
    $irrigation->add_channel('mutant/%/squirrels', $this, 'flee');

    $channel = $irrigation->find_channel('warrior/resurrect');
    $this->assertEquals('object', gettype($channel));
    $this->assertEquals('resurrect', $channel->method);

    $this->assertNull($irrigation->find_channel('warrior'));
    $this->assertNull($irrigation->find_channel('warrior/resurrect/nothing'));
    $this->assertNull($irrigation->find_channel('warrior/resurrect2'));

    $channel = $irrigation->find_channel('harvest');
    $this->assertEquals('object', gettype($channel));
    $this->assertEquals('harvest_food', $channel->method);

    $this->assertNotNull($irrigation->find_channel('mutant/killer/squirrels'));
    $this->assertNull($irrigation->find_channel('mutant/squirrels'));
    $this->assertNull($irrigation->find_channel('mutant/killer'));
  }

  function test_service() {
    $temp = new Temp();
    $this->fixture->load_schemas();
    $irrigation = $this->ground->irrigation = new Irrigation($this->ground);
    $irrigation->add_channel('kill/%', $temp, 'nothing');
    $request = new Bag('kill/time');
    $this->ground->vineyard_service($request);
    $this->assertTrue($temp->called);
  }

}
