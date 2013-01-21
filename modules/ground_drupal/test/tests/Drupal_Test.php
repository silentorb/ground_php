<?php

class Drupal_Test extends Drupal_Test_Fixtures {
  function setUp() {
    $this->ground = new Ground('ground_test');
    $this->ground->add_module('Ground_Drupal');
    $this->prepare_database();
  }

  function test_users() {
    $this->insert_object('user', array(
        'name' => 'Bob',
        'email' => 'nothing@nowhere.com',
        'pass' => 'password',
    ));

    $query = $this->ground->create_query('user');
    $objects = $query->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals('Bob', $objects[0]->name);
  }

  function test_nodes() {
    $this->insert_object('node', array(
        'title' => 'A Node',
        'type' => 'test',
        'body' => '<i>This is the body</i>',
    ));

    $query = $this->ground->create_query('node');
    $objects = $query->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals('A Node', $objects[0]->title);
  }

}
