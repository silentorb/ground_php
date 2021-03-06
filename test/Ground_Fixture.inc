<?php

class Ground_Fixture extends Fixture {
  function load_schemas() {
    $path = drupal_get_path('module', 'ground_php');
    $this->ground->load_schema_from_file($path . '/vineyard.json');
    $this->ground->load_schema_from_file($path . '/test/test-trellises.json');
    $this->test->assertArrayHasKey('warrior', $this->ground->trellises);
    $this->trellis = $this->ground->trellises['warrior'];
  }

  function prepare_database() {
    parent::prepare_database();
    $this->insert_custom_cross_table();
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

  function insert_custom_cross_table() {
    $db = $this->ground->db;
    // Add cross table to database.  It's not automatically added because it has no trellis.
    $table = $this->ground->tables['branches_deeds'];
    if ($table) {
      $sql = $table->create_sql();
      $db->query($sql);
    }
  }

  function add_roles_users() {
    $db = $this->ground->db;
    $table = $this->ground->tables['roles_users'];
    $db->query($table->create_sql());
  }

}
