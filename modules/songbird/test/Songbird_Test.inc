<?php

class Songbird_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();

    $this->fixture->load_schemas();
    $this->songbird = $this->ground->add_module('Songbird');
    $this->fixture->prepare_database();

    $this->ground->insert_object('song_type', array(
        'name' => 'blossom'
    ));

    $this->ground->insert_object('user', array(
        'uid' => 7,
        'name' => 'cj',
        'email' => 'nothing@nowhere.com',
    ));

    $this->ground->insert_object('user', array(
        'uid' => 8,
        'name' => 'shambler',
        'email' => 'shambler@nowhere.com',
    ));

    $db = $this->ground->db;
    $table = $this->ground->tables['songs_users'];
    $db->query($table->create_sql());
  }

  function test_sing() {
    $songbird = $this->songbird;

    $data = 'This will usually be JSON.';
    $listeners = array(7, 8);
    $songbird->sing('blossom', $data, 7, $listeners);

    $objects = $this->ground->create_query('song')->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals($data, $objects[0]->data);
    $this->assertEquals('blossom', $objects[0]->type->name);
    $this->assertEquals('shambler', $objects[0]->listeners[1]->name);
  }

  function test_query_songs() {
    $irrigation = $this->ground->irrigation = new Irrigation($this->ground);
    $songbird = $this->songbird;
    $listeners = array(7, 8);
    $songbird->sing('blossom', '1', 7, $listeners);
    $songbird->sing('blossom', '2', 7, $listeners);
    $db = $this->ground->db;
    $db->query("UPDATE songs SET `timestamp` = 50 WHERE data = '1'");
    $db->query("UPDATE songs SET `timestamp` = 100 WHERE data = '2'");

    Irrigation::import();
    $request = new Bag('vineyard/song');
    $request->arguments = array(
        'start' => 40,
        'end' => 50,
    );
    $result = $irrigation->service($request);
    $objects = $result->objects;
    $this->assertEquals(1, count($objects));
    $this->assertEquals('1', $objects[0]->data);

    $request->arguments = array(
        'start' => 100,
        'end' => 110,
    );
    $result = $irrigation->service($request);
    $objects = $result->objects;
    $this->assertEquals(1, count($objects));
    $this->assertEquals('2', $objects[0]->data);
  }

}
