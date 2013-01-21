<?php

module_load_include('inc', 'ground_php', 'test/Fixtures');

class Ground_Test_Fixtures extends Fixtures {

  public $modules = array();

  function setUp() {
    $this->ground = new Ground('ground_test');
    $this->ground->db->drop_all_tables();
  }

  function fixture_load_schemas() {
    foreach ($this->modules as $module) {
      $this->ground->add_module($module);
    }
    $path = drupal_get_path('module', 'ground_php');
    $this->ground->load_schema_from_file($path . '/test/test-trellises.json');
    $this->assertArrayHasKey('warrior', $this->ground->trellises);
    $this->trellis = $this->ground->trellises['warrior'];
  }

  function fixture_populate_database() {
    $this->fixture_load_schemas();
    $this->prepare_database();

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

}
