<?php

class Ground_Test_Fixtures extends PHPUnit_Framework_TestCase {
  function fixture_load_schemas() {
    $this->ground = new Ground('ground_test');
    $path = drupal_get_path('module', 'ground_php');
    $this->ground->load_schema_from_file($path . '/test/test-trellises.json');
    $this->trellis = $this->ground->trellises['warrior'];
  }

  function fixture_populate_database() {
    $this->fixture_load_schemas();
    $db = $this->ground->db;
    $db->drop_all_tables();
    $db->create_tables($this->ground->trellises);

    $ninja_bob = $this->insert_object('warrior', array(
        'name' => 'Bob',
            ));

    $this->insert_object('character_item', array(
        'name' => 'sword',
        'owner' => $ninja_bob->id,
    ));

//    $this->trellis = $this->ground->trellises['warrior'];
//    $this->object = $this->trellis->create_object();
//    $update = new Update($this->trellis, $this->object, $this->ground);
//    $result = $update->run();
  }

  function insert_object($trellis_name, $data = array()) {
    $trellis = $this->ground->trellises[$trellis_name];
    $object = $trellis->create_object();
    MetaHub::extend($object, $data);
    $update = new Update($trellis, $object, $this->ground);
    $result = $update->run();
    return $object;
  }

}
