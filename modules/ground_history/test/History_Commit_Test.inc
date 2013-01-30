<?php

class History_Commit_Test extends Ground_Test_Fixtures {
  function setUp() {
    parent::setUp();
    $this->ground->add_module('History');
    $this->fixture_populate_database();
  }

  function test_commit() {
    $commit = new History_Commit($this->ground);
    $commit->process_object($this->ninja_bob, $this->trellis);
    $this->assertEquals(3, count($commit->updates));
    $this->assertEquals($this->ninja_bob->age, $commit->updates[2]->value);
    $commit->save();

    $query = $this->ground->create_query('history_update');
    $objects = $query->run();
    $this->assertEquals(count($commit->updates), count($objects));
    $this->assertEquals($this->ninja_bob->race, $objects[1]->value);
    $this->assertEquals($this->ninja_bob->age, $objects[2]->value);

    $query = $this->ground->create_query('history_object');
    $objects = $query->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals('warrior', $objects[0]->trellis);
    $this->assertSame(1, $objects[0]->object_id);
  }

  function test_modify() {
    $commit = new History_Commit($this->ground);
    $commit->process_object($this->ninja_bob, $this->trellis);
    $commit->save();

    $commit = new History_Commit($this->ground);
    $this->ninja_bob->race = 'legendary sage';
    $commit->process_object($this->ninja_bob, $this->trellis);
    $this->assertSame(1, count($commit->history->objects));
    $this->assertSame(1, $commit->history->objects['warrior.1']->id);

    $commit->save();

    $query = $this->ground->create_query('history_update');
    $objects = $query->run();
    $this->assertEquals(4, count($objects));
  }

}
