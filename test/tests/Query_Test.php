<?php

class Query_Test extends Ground_Test_Case {
  function test_select() {
    $this->fixture->populate_database();
    $query = $this->ground->create_query($this->ground->trellises['warrior']);
    $objects = $query->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals('Bob', $objects[0]->name);

    $query = $this->ground->create_query($this->ground->trellises['character_item']);
    $objects = $query->run();
    $this->assertEquals(1, $objects[0]->owner->id);
  }

  // Originally Vineyard was designed to only support links that had explicit definitions within both
  // Trellises, but it has become increasingly useful when embedding into existing systems not to need
  // to insert the connection definition in the pre-existing table.  Note that these new 'implicit'
  // connections only work in one direction, you still need to define both sides of the connection to
  // access it from either direction.
  function test_one_way_reference() {
    $this->fixture->populate_database();
    // Warrior has no reciprical connection to achievement.
    $this->fixture->insert_object('achievement', array(
        'name' => 'Slay 10000 Hoarse Radishes',
        'parent' => $this->fixture->ninja_bob->id,
    ));

    $objects = $this->ground->create_query('achievement')->run();
    $this->assertEquals(1, count($objects));
    $this->assertEquals('Bob', $objects[0]->parent->name);
  }

  private function prepare_tree() {
    $this->fixture->load_schemas();
    $this->fixture->prepare_database();

    // Kind of confusing looking but it actually makes sense.  Looks cleaner in JSON.
    $this->fixture->insert_object('branch', array(
        'name' => 'A',
        'children' => array(
            array(
                'name' => 'B',
                'children' => array(
                    array(
                        'name' => 'C',
                    )
                )
            )
        )
    ));
  }

  function test_has_expansion() {
    $this->fixture->load_schemas();
    $this->ground->expansions[] = '/.*images_fid/';
    $query = $this->ground->create_query('warrior');
    $this->assertTrue($query->has_expansion('object/images_fid'));
  }

  function test_static_queries() {
    $this->fixture->populate_database();
    $this->fixture->insert_object('warrior', array(
        'name' => 'Frank',
        'race' => 'detective',
        'age' => 25,
    ));
    $template = $this->ground->create_static_query('test_query', 'warrior');
    $template->add_filter("race = 'detective'");
    $objects = $this->ground->create_query('test_query')->run();
    $this->assertSame(1, count($objects));
    $this->assertEquals('Frank', $objects[0]->name);
  }

  function test_tree() {
    $this->prepare_tree();
    $query = $this->ground->create_query('branch');
    $query->add_filter('branches.id = 1');
    $objects = $query->run();
    $this->assertSame(1, count($query));
    $this->assertEquals('B', $objects[0]->children[0]->name);
    $this->assertEquals(0, count($objects[0]->children[0]->children));
  }

  function test_tree_part_two() {
    $this->prepare_tree();

    $this->ground->expansions[] = 'dummy/test';
    $this->ground->expansions[] = 'branch/children/children';
    $query = $this->ground->create_query('branch');
    $query->add_filter('branches.id = 1');
    $objects = $query->run();
    $this->assertSame(1, count($query));
    $this->assertEquals('B', $objects[0]->children[0]->name);
    $this->assertEquals('C', $objects[0]->children[0]->children[0]->name);
    $this->assertEquals('2', $objects[0]->children[0]->children[0]->parent);
  }

}
