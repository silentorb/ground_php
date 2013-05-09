<?php

class Temp {
  function nothing() {
    $this->called = true;
    return array(
        'first item'
    );
  }

}

class Irrigation_Test extends Ground_Test_Case {
  function test_get_simple_url_path() {
    $this->assertEquals('dragon/lair', Irrigation::get_simple_url_path('dragon/lair'));
    $this->assertEquals('dragon/lair', Irrigation::get_simple_url_path('dragon/lair?treasure=10021903'));
  }

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

  function test_prepare_input() {
    $parameters = array(
        'fish' => array(
            'type' => 'int',
        ),
        'toad' => array(
            'type' => 'string',
            'default' => 'Mr. Mosspuddle',
        ),
        'speed' => array(
            'type' => 'double',
        ),
        'cat' => array(
            'type' => 'bool',
        ),
    );

    $arguments = array(
        'fish' => '20',
        'speed' => 20.3,
    );
    
    $result =Irrigation::prepare_input($parameters, $arguments);
    $this->assertSame(20, $result['fish']);
    $this->assertEquals('Mr. Mosspuddle', $result['toad']);
    $this->assertSame(20.3, $result['speed']);
    $this->assertFalse(isset($result['cat']));
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
    $result = $this->ground->vineyard_service($request);
    $this->assertTrue($temp->called);
    $this->assertEquals('first item', $result->objects[0]);

    $request = new Bag('vineyard/kill/time');
    $this->ground->vineyard_service($request);
    $this->assertTrue($temp->called);
  }

  function test_dynamic_service_filters() {
    $this->fixture->populate_database();
    $irrigation = $this->ground->irrigation = new Irrigation($this->ground);
    $this->fixture->insert_object('warrior', array(
        'name' => 'Frank',
        'race' => 'detective',
        'age' => 18,
    ));

    $request = new Bag('warrior');
    $request->arguments['name'] = 'Frank';
    $result = $this->ground->vineyard_service($request);
    $this->assertSame(1, count($result->objects));
  }

}
