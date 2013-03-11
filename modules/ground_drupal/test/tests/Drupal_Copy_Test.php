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

  function test_fixture() {
    $query = new Content_Query($this->drupal_ground->trellises['node']);
    $query->add_post('ORDER BY node.nid');
    $objects = $query->run();
    $this->assertEquals('array', gettype($objects[0]->images));
    $this->assertEquals('object', gettype($objects[0]->images[0]));
  }

  function test_get_nodes() {
    Ground_Drupal::configure_files_expansion($this->drupal_ground);
    $query = new Content_Query($this->drupal_ground->trellises['node']);
    $query->add_post('ORDER BY nid');
    $objects = $query->run();
    $this->assertSame(false, $objects[2]->location->is_primary);
    $this->assertGreaterThan(1, count($objects[0]->images));
    $this->assertEquals('object', gettype($objects[0]->images[0]->images_fid));
  }

  function test_transfer() {
    $db = $this->ground->db;
    $result = $db->query_values("SHOW TABLES LIKE 'content_field_victims'");
    $this->assertEquals(1, count($result));

    Update::$log_queries = true;
    Ground_Drupal::copy_nodes($this->drupal_ground, $this->ground);
    $query = new Content_Query($this->ground->trellises['node']);
    $query->add_post('ORDER BY nid');
    $objects = $query->run();
    $this->assertEquals(3, count($objects));
    $this->assertGreaterThan(0, count($objects[0]->images));

    // Check monsters
    $count = $db->query_value('SELECT COUNT(*) FROM content_type_monster');
    $this->assertEquals(2, $count);

    // Check files
    $count = $db->query_value('SELECT COUNT(*) FROM files');
    $this->assertEquals(5, $count);

    // Check locations
    $count = $db->query_value('SELECT COUNT(*) FROM location');
    $this->assertEquals(1, $count);

    $vid = $db->query_value('SELECT vid FROM `node` WHERE nid = 3');
    $this->assertEquals(4, $vid);

    $field_friend_nid = $db->query_value('SELECT field_friend_nid FROM `content_field_friend` WHERE nid = 1');
    $this->assertEquals(2, $field_friend_nid);
  }

}
