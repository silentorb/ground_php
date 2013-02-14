<?php

class Fixtures extends PHPUnit_Framework_TestCase {
  protected $backupGlobals = FALSE;
  
  function prepare_database() {
    $db = $this->ground->db;
    $db->drop_all_tables();
    print count($this->ground->trellises);
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
