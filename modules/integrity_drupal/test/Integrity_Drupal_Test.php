<?php

class Integrity_Drupal_Test extends Ground_Test_Case {
  function setUp() {
    parent::setUp();
    $this->integrity_drupal = $this->ground->add_module('Integrity_Drupal');
  }

  function test() {
    $query = $ground->create_query('node');
    $objects = $query->run();
    $this->assertEquals(count($objects), 2);
  }

}
