<?php

class History_Commit_Test extends Ground_Test_Fixtures {
  function setUp() {
    $this->modules[] = 'History';
    $this->fixture_populate_database();
  }

  function test_commit() {
    $commit = new History_Commit($this->ground);
    $commit->process_object($this->ninja_bob, $this->trellis);
    $this->assertEquals(3, count($commit->updates));
    $this->assertEquals($this->ninja_bob->age, $commit->updates[1]->value);
    $commit->save();

    $query = $this->ground->create_query('history_update');
    $objects = $query->run();
    $this->assertEquals(count($commit->updates), count($objects));
    $this->assertEquals($this->ninja_bob->race, $objects[0]->value);
    $this->assertEquals($this->ninja_bob->age, $objects[1]->value);
  }

}
