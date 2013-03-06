<?php

class Drupal_Copy_Test extends Drupal_Test_Case {
  function setUp() {
    parent::setUp();
    $this->ground->add_module('Ground_Drupal');
    $drupal_ground = $this->drupal_ground = new Ground('ground_drupal');
    $this->drupal = $drupal_ground->add_module('Ground_Drupal');
    $this->cck = $drupal_ground->add_module('CCK');
    $this->cck->add_content_types($this->drupal_ground);
    $this->cck->add_content_types($this->ground);
    $this->fixture->prepare_database();
  }

  function test_cross_table() {
    $db = $this->ground->db;
    $result = $db->query_values("SHOW TABLES LIKE 'content_field_victims'");
    $this->assertEquals(1, count($result));
  }
  
  function test() {
    $db = $this->ground->db;
    Update::$log_queries = true;
    Ground_Drupal::copy_nodes($this->drupal_ground, $this->ground);
    $query = new Content_Query($this->ground->trellises['node']);
    $objects = $query->run();
    $this->assertEquals(2, count($objects));
    $this->assertEquals(1, $objects[0]->image_list);

    $count = $db->query_value('SELECT COUNT(*) FROM content_type_monster');
    $this->assertEquals(2, $count);
  }


}
