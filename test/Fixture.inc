<?php

class Fixture {
  public function __construct($database_name, $test = null) {
    $this->ground = new Ground($database_name);
    if (is_object($test)) {
      $test->ground = $this->ground;
      $this->test = $test;
    }
  }

  function prepare_database() {
    $db = $this->ground->db;
    $db->drop_all_tables();
    $db->create_tables($this->ground->trellises);
  }

  function insert_object($trellis, $data = array()) {
    if (is_string($trellis)) {
      $trellis_name = $trellis;
      $trellis = $this->ground->trellises[$trellis_name];
      if (!$trellis)
        throw new Exception("Trellis $trellis_name not found.");
    }

    $object = $trellis->create_object();
    MetaHub::extend($object, $data);
    $update = new Update($trellis, $object, $trellis->ground);
    $result = $update->run();
    return $object;
  }

}
