<?php

class CCK_Test extends Drupal_Test_Case {
  function setUp() {
    parent::setUp();
    $this->fixture->prepare_database();
    $this->ground->add_module('Ground_Drupal');
    $drupal_ground = $this->drupal_ground = new Ground('ground_drupal');
    $this->drupal = $drupal_ground->add_module('Ground_Drupal');
    $this->cck = $drupal_ground->add_module('CCK');
  }

  function test_read_content_types() {
    $types = $this->cck->read_content_types();
    $this->assertArrayHasKey('monster', $types);
  }

  function test_read_cross_tables() {
    $types = $this->cck->read_cross_tables();
    $this->assertArrayHasKey('victims', $types);
  }

  function test_convert_cck_joins_to_trellises() {
    $types = $this->cck->read_cross_tables();
    $result = $this->cck->convert_cck_joins_to_trellises($types, $this->ground);

    $this->assertArrayHasKey('victims', $result['trellises']);
    $this->assertArrayNotHasKey('', $result['trellises']['images']->properties);
    $this->assertArrayHasKey('images_data', $result['trellises']['images']->properties);
    $this->assertArrayHasKey('victims', $this->ground->trellises);

    // Second Level Reference
    $images = $result['trellises']['images'];
    $this->assertEquals('reference', $images->properties['images_fid']->type);
    $this->assertEquals('file', $images->properties['images_fid']->trellis);
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

    // Table
    $this->assertArrayHasKey('monster', $result['tables']);
    $this->assertArrayHasKey('teeth', $result['tables']['monster']->properties);
    $this->assertEquals('field_teeth_value', $result['tables']['monster']->properties['teeth']->name);
    $this->assertEquals('field_doc_fid', $result['tables']['monster']->properties['doc_fid']->name);
  }

  function test_add_content_types() {
    $this->cck->add_content_types($this->ground);
    $this->assertArrayHasKey('monster', $this->ground->trellises);
  }

  function test_cck_cross_table() {
    $this->cck->add_content_types($this->ground);
    $this->ground->db->create_tables($this->cck->trellises);
    $trellis = $this->ground->trellises['monster'];
    $property = $trellis->properties['victims'];
    $this->assertNotNull($property);
    $this->assertNotNull($trellis->table);
    $this->assertArrayHasKey('victims', $trellis->properties);

//    $this->assertArrayHasKey('victims', $trellis->table->properties);
//    $this->assertEquals('CCK_Link', $trellis->table->properties['victims']->link_class);
//    $this->assertEquals('CCK_Link', $property->get_link_class());
  }

  // Was having issues where Query was thinking lair was 1-to-many instead of many-to-many
  function test_lair() {
    $this->cck->add_content_types($this->ground);
    $trellis = $this->ground->trellises['monster'];
    $property = $trellis->properties['lair'];
    $this->assertNotNull($property);
    $other_property = $property->get_other_property();
    $this->assertEquals('list', $other_property->type);
  }

  function test_add_objects() {
    $this->fixture->prepare_database();
    $this->cck->add_content_types($this->ground);
    $this->ground->db->create_tables($this->cck->trellises);
    $this->assertTrue(Table::exists($this->ground->db, 'content_field_victims'));

    Ground_Drupal::add_user($this->ground, 'Ninja Bird', 'secret', 'ninja@nest.com');

    $this->fixture->insert_object('lair', array(
        'title' => 'The Dark Cave',
    ));

    $this->ogre = $this->fixture->insert_object('monster', array(
        'title' => 'Ugh',
        'teeth' => 12,
        'lair' => 1,
            ));

    $query = $this->ground->create_query('monster');
    $monsters = $query->run();
    $this->assertSame(1, count($monsters));
    $this->assertEquals(12, $monsters[0]->teeth);
    $this->assertEquals('Ugh', $monsters[0]->title);
//    $this->assertSame(1, count($monsters[0]->victims));
  }

}
