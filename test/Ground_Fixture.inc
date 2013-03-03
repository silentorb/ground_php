<?php

class Ground_Fixture extends Fixture {
  
  function load_schemas() {
//    foreach ($this->modules as $module) {
//      $this->ground->add_module($module);
//    }
    $path = drupal_get_path('module', 'ground_php');
    $this->ground->load_schema_from_file($path . '/test/test-trellises.json');
    $this->test->assertArrayHasKey('warrior', $this->ground->trellises);
    $this->trellis = $this->ground->trellises['warrior'];
  }

  function populate_database() {
    $this->load_schemas();
    $this->prepare_database();

    $this->ninja_bob = $this->insert_object('warrior', array(
        'name' => 'Bob',
        'race' => 'legendary',
        'age' => 31,
            ));

    $this->test->assertSame(1, $this->ninja_bob->id);

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