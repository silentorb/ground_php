<?php

class Brick_Test extends Ground_Test_Case {
  function setUp() {
    $this->fixture = new Fortress_Fixture($this->database, $this);
//    $this->ground->db->drop_all_tables();
//    $this->ground->add_module('Ground_Drupal');
  }

  public function test_admin_brick() {
    $fortress = new Fortress($this->ground);
    $fortress->lay_brick(new Admin_Gate());

    $resource = (object) array(
            'trellis' => 'ninja pig',
            'owner' => 5,
    );

    $shambler = (object) array(
            'uid' => 8,
    );

    $this->assertSame(0, $fortress->access($shambler, $resource));
    $shambler->roles = array('admin');
    $this->assertSame(3, $fortress->access($shambler, $resource));

    // At first I felt I was being over-the-top writing this variation
    // of the test, but it actually broke and revealed a bug with Admin_Brick initialization!
    $fortress = new Fortress($this->ground);
    $fortress->lay_brick(new Admin_Gate('king'));
    $this->assertSame(0, $fortress->access($shambler, $resource));
    $shambler->roles = array('king');
    $this->assertSame(3, $fortress->access($shambler, $resource));
  }

  public function test_public_trellis_brick() {
    $fortress = new Fortress($this->ground);

    $resource = (object) array(
            'trellis' => 'ninja pig',
    );
    $account = (object) array(
            'uid' => 8,
    );

    $this->assertSame(0, $fortress->access($account, $resource));
    $fortress->lay_brick(new Public_Trellis_Gate(array('frog')));
    $this->assertSame(0, $fortress->access($account, $resource));
    $fortress->lay_brick(new Public_Trellis_Gate(array('ninja pig')));
    $this->assertSame(1, $fortress->access($account, $resource));
    $this->assertSame(1, $fortress->access($account, new stdClass(), array('trellis' => 'ninja pig')));
  }

  public function test_owner_brick() {
    $fortress = new Fortress($this->ground);

    $resource = (object) array(
            'trellis' => 'ninja pig',
            'owner' => 5,
    );
    $farmer = (object) array(
            'uid' => 5,
    );
    $shambler = (object) array(
            'uid' => 8,
    );

    $this->assertSame(0, $fortress->access($farmer, $resource));
    $this->assertSame(0, $fortress->access($shambler, $resource));
    $fortress->lay_brick(new Owner_Gate());
    $this->assertSame(3, $fortress->access($farmer, $resource));
    $this->assertSame(0, $fortress->access($shambler, $resource));
  }
  
}
