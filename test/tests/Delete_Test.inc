<?php

class Delete_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();
    $this->fixture->populate_database();
  }

  function test_delete_using_class() {
    $db = $this->ground->db;
    $delete = new Delete();
    $delete->run($this->ground->trellises['warrior'], $this->fixture->ninja_bob);

    $count = $db->query_value('SELECT COUNT(*) FROM base_objects');
    $this->assertEquals(0, $count, 'base_objects');

    $count = $db->query_value('SELECT COUNT(*) FROM warriors');
    $this->assertEquals(0, $count, 'warriors');

    $objects = $this->ground->create_query('warrior')->run();
    $this->assertEquals(0, count($objects), 'full query');
  }

  function test_delete_using_ground() {
    $this->fixture->ninja_bob->_deleted = true;
    $trellis = $this->ground->trellises['warrior'];
    $this->ground->update_object($trellis, $this->fixture->ninja_bob);
    $objects = $this->ground->create_query('warrior')->run();
    $this->assertEquals(0, count($objects));
  }

  function test_delete_aggregation_methods() {
    $delete = new Delete();
    $warrior_trellis = $this->ground->trellises['warrior'];
    $links = $delete->get_child_links($warrior_trellis);
    $this->assertEquals(1, count($links));
  }

  function test_delete_with_children() {
    $this->fixture->insert_object('achievement', array(
        'name' => 'Slay 10000 Hoarse Radishes',
        'parent' => $this->fixture->ninja_bob->id,
    ));

    $this->fixture->insert_object('deed', array(
        'name' => 'Rescued a fair maiden.',
        'parent' => $this->fixture->ninja_bob->id,
    ));

    $objects = $this->ground->create_query('achievement')->run();
    $this->assertEquals(1, count($objects));

    $db = $this->ground->db;
    $delete = new Delete();
    $delete->run($this->ground->trellises['warrior'], $this->fixture->ninja_bob);

    // Achievements are a one way reference to warrior, so a warrior should not be able to see
    // achievements and know to delete them.
    $objects = $this->ground->create_query('achievement')->run();
    $this->assertEquals(1, count($objects), 'achievement was not deleted.');

    // Deeds are a two way reference to warrior so warrior should see the deeds and
    // delete them when a warrior is deleted.
    $objects = $this->ground->create_query('deed')->run();
    $this->assertEquals(0, count($objects), 'deed was deleted.');

    // Double check the individual tables.
    $count = $db->query_value('SELECT COUNT(*) FROM base_objects');
    $this->assertEquals(1, $count, 'base_objects');

    $count = $db->query_value('SELECT COUNT(*) FROM warriors');
    $this->assertEquals(0, $count, 'warriors');

    $count = $db->query_value('SELECT COUNT(*) FROM achievements');
    $this->assertEquals(1, $count);

    $count = $db->query_value('SELECT COUNT(*) FROM deeds');
    $this->assertEquals(0, $count);
  }

}
