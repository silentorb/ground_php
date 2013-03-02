<?php

class Drupal_Test extends Drupal_Test_Case {
  function setUp() {
    parent::setUp();
    $this->ground->add_module('Ground_Drupal');
    $this->fixture->prepare_database();
  }

  function test_users() {
    $this->fixture->insert_object('user', array(
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
    $this->fixture->insert_object('node', array(
        'title' => 'A Node',
        'type' => 'test',
        'body' => '<i>This is the body</i>',
    ));

    $query = $this->ground->create_query('node');
    $objects = $query->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals('A Node', $objects[0]->title);
  }

  function test_add_user() {
    Ground_Drupal::add_user($this->ground, 'Ninja Bird', 'secret', 'ninja@nest.com');
    $query = $this->ground->create_query('user');
    $objects = $query->run();

    $this->assertEquals(1, count($objects));
    $this->assertEquals('Ninja Bird', $objects[0]->name);
    $this->assertEquals('ninja@nest.com', $objects[0]->email);
    $this->assertEquals(md5('secret'), $objects[0]->password);
  }

}
