<?php

class Ground_Test_Fixtures extends PHPUnit_Framework_TestCase {
  public $modules = array();
  
  function fixture_load_schemas() {
    $this->ground = new Ground('ground_test');
    foreach($this->modules as $module) {
      $this->ground->add_module($module);
    }
    $path = drupal_get_path('module', 'ground_php');
    $this->ground->load_schema_from_file($path . '/test/test-trellises.json');
    $this->assertArrayHasKey('warrior', $this->ground->trellises);
    $this->trellis = $this->ground->trellises['warrior'];
  }

  function fixture_populate_database() {
    $this->fixture_load_schemas();
    $db = $this->ground->db;
    $db->drop_all_tables();
    $db->create_tables($this->ground->trellises);

    $this->ninja_bob = $this->insert_object('warrior', array(
        'name' => 'Bob',
        'race' => 'legendary',
        'age' => 31,
            ));
    
    $this->assertSame(1, $this->ninja_bob->id);

    $this->insert_object('character_item', array(
        'name' => 'sword',
        'owner' => $this->ninja_bob->id,
    ));

//    $this->trellis = $this->ground->trellises['warrior'];
//    $this->object = $this->trellis->create_object();
//    $update = new Update($this->trellis, $this->object, $this->ground);
//    $result = $update->run();
  }

  function insert_object($trellis_name, $data = array()) {
    $trellis = $this->ground->trellises[$trellis_name];
    $this->assertArrayHasKey($trellis_name, $this->ground->trellises);
    $object = $trellis->create_object();
    MetaHub::extend($object, $data);
    $update = new Update($trellis, $object, $this->ground);
    $result = $update->run();
    return $object;
  }

}
