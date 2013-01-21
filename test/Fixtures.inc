<?php

class Fixtures extends PHPUnit_Framework_TestCase {

  function prepare_database() {
    $db = $this->ground->db;
    $db->drop_all_tables();
    $db->create_tables($this->ground->trellises);
  }

  function insert_object($trellis_name, $data = array()) {
    $trellis = $this->ground->trellises[$trellis_name];
    if (!$trellis)
      throw new Exception("Trellis $trellis_name not found.");
    
    $object = $trellis->create_object();
    MetaHub::extend($object, $data);
    $update = new Update($trellis, $object, $this->ground);
    $result = $update->run();
    return $object;
  }
}
