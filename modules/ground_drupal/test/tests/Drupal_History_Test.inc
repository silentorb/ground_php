<?php

class Drupal_History_Test extends Drupal_Test_Case {
  function setUp() {
    parent::setUp();
    $this->fixture->prepare_database();
    $this->ground->add_module('Ground_Drupal');
    $drupal_ground = $this->drupal_ground = new Ground('ground_drupal');
    $this->drupal = $drupal_ground->add_module('Ground_Drupal');
    $this->cck = $drupal_ground->add_module('CCK');
  }
  
  function test_add_objects() {
    $this->fixture->prepare_database();
    $this->cck->add_content_types($this->ground);
    $this->ground->db->create_tables($this->cck->trellises);

    
  }

}
