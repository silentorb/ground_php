<?php

class Drupal_Test extends Drupal_Test_Case {
  function setUp() {
    parent::setUp();
    $this->ground->add_module('Ground_Drupal');
    $this->fixture->prepare_database();
    $this->fixture->add_roles_users();
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

  function test_create_node() {
    // Grab custom node types such as 'monster'.
    $drupal_ground = $this->drupal_ground = new Ground('ground_drupal');
    $this->drupal = $drupal_ground->add_module('Ground_Drupal');
    $cck = $drupal_ground->add_module('CCK');
    $cck->add_content_types($this->ground);
    $this->fixture->prepare_database();

    $data = array(
        'type' => 'monster',
        'title' => 'Giant Slug',
        'body' => 'Really big and disgusting.',
        'uid' => 7,
    );

    $node = Ground_Drupal::create_node($this->ground, $data);
    // Right null is the expected result.
    $this->assertNull($node->status);
    $this->assertEquals('Giant Slug', $node->title);
    $this->assertEquals('Really big and disgusting.', $node->body);
    $this->assertEquals('monster', $node->type);

    $objects = $this->ground->create_query('monster')->run();
    $node = $objects[0];

    $this->assertEquals(1, $node->status);
    $this->assertEquals('Giant Slug', $node->title);
//    $this->assertEquals('Really big and disgusting.', $node->body);
    $this->assertEquals('monster', $node->type);
  }

  // Ensure that vids can not be externally modified.
  function test_no_updating_private_fields() {
    $node = $this->ground->insert_object('node', array(
        'title' => 'A Node',
        'type' => 'test',
        'body' => '<i>This is the body</i>',
        ));

    $this->assertEquals(1, $node->vid);

    $trellis = $this->ground->trellises['node'];
    $node->vid = 128;
    $this->ground->update_object_with_service($trellis, $node);
    $node = $this->ground->create_query('node')->run_as_service();
    $this->assertNull($node->vid);
    $vid = $this->ground->db->query_value("SELECT vid FROM node");
    $this->assertEquals(1, $vid);

    $node->vid = 0;
    $this->ground->update_object_with_service($trellis, $node);
    $vid = $this->ground->db->query_value("SELECT vid FROM node");
    $this->assertEquals(1, $vid);

  }

}
