<?php

class CCK_Test extends Drupal_Test_Fixtures {
  function setUp() {
    $this->ground = new Ground('ground_test');
    $this->ground->add_module('Ground_Drupal');
    $this->prepare_database();
    $drupal_ground = $this->drupal_ground = new Ground('ground_drupal');
    $this->drupal = $drupal_ground->add_module('Ground_Drupal');
    $this->cck = $drupal_ground->add_module('CCK');
  }

  function test_read_content_types() {
    $types = $this->cck->read_content_types();
    $this->assertArrayHasKey('monster', $types);
  }

  function test_convert_cck_to_trellises() {
    $types = $this->cck->read_content_types();
    $result = $this->cck->convert_cck_to_trellises($types, $this->ground);
    $this->assertArrayHasKey('monster', $result['trellises']);
    $trellis = $result['trellises']['monster'];
    $this->assertNotNull($trellis->table);
    $this->assertEquals('content_type_monster', $trellis->get_table_name());
    $this->assertEquals('content_type_monster', $trellis->get_table_name());
    $this->assertGreaterThan(0, count($trellis->properties));
  }

  function test_add_content_types() {
    $this->cck->add_content_types($this->ground);
    $this->assertArrayHasKey('monster', $this->ground->trellises);
  }

  function test_override_link_class() {
    $this->cck->add_content_types($this->ground);
    $this->ground->db->create_tables($this->cck->trellises);
    $trellis = $this->ground->trellises['monster'];
    $property = $trellis->properties['victims'];
    $this->assertNotNull($trellis->table);
    $this->assertArrayHasKey('victims', $trellis->table->properties);
    $this->assertEquals('CCK_Link', $trellis->table->properties['victims']->link_class);
    $this->assertEquals('CCK_Link', $property->get_link_class());
  }

  function test_custom_link_class() {
    $this->cck->add_content_types($this->ground);
    $trellis = $this->ground->trellises['monster'];
    $property = $trellis->properties['victims'];
    $link = new CCK_Link($property);
    $args = $link->get_arguments($property);
    $this->assertEquals('content_field_victims', $args['%table_name']);
    $this->assertEquals('field_victims_uid', $args['%first_key']);
    $this->assertEquals('nid', $args['%second_key']);
    $this->assertEquals('content_type_monster.nid', $args['%first_id']);
    $this->assertEquals('users.uid', $args['%second_id']);
  }

  // Was having issues where Query was thinking lair was 1-to-many instead of many-to-many
  function test_lair() {
    $this->cck->add_content_types($this->ground);
    $trellis = $this->ground->trellises['monster'];
    $property = $trellis->properties['lair'];
    $other_property = $property->get_other_property();
    $this->assertEquals('list', $other_property->type);
  }

  function test_add_objects() {
    $this->cck->add_content_types($this->ground);
    $this->ground->db->create_tables($this->cck->trellises);
    Ground_Drupal::add_user($this->ground, 'Ninja Bird', 'secret', 'ninja@nest.com');

    $this->insert_object('lair', array(
        'title' => 'The Dark Cave',
    ));

    $this->ogre = $this->insert_object('monster', array(
        'title' => 'Ugh',
        'teeth' => 12,
        'lair' => 1,
        'victims' => array(1),
            ));

    $query = $this->ground->create_query('monster');
    $monsters = $query->run();
    $this->assertSame(1, count($monsters));
    $this->assertEquals(12, $monsters[0]->teeth);
    $this->assertEquals('Ugh', $monsters[0]->title);
    $this->assertSame(1, count($monsters[0]->victims));
  }

}
