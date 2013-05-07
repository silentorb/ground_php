<?php

class Brick_Test extends Ground_Test_Case {
  function setUp() {
    $this->fixture = new Fortress_Fixture($this->database, $this);
//    $this->ground->db->drop_all_tables();
//    $this->ground->add_module('Ground_Drupal');
  }

}
