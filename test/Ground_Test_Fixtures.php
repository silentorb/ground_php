<?php

class Ground_Test_Fixtures extends PHPUnit_Framework_TestCase {
  function fixture_populated_database() {
    $this->ground = new Ground('ground_test');
    $path = drupal_get_path('module', 'ground_php');
    $this->ground->load_schema_from_file($path . '/test/test-trellises.json');
    $this->trellis = $this->ground->trellises['warrior'];
    $this->object = $this->trellis->create_object();
    $this->object->name = 'Bob';
    $db = $this->ground->db;
    $db->drop_all_tables();
    $db->create_tables($this->ground->trellises);
    $update = new Update($this->trellis, $this->object, $this->ground);
    $result = $update->run();
  }

}
