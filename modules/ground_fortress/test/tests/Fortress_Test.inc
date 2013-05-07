<?php

class Fortress_Test extends Ground_Test_Case {
  function setUp() {
    $this->fixture = new Fortress_Fixture($this->database, $this);
    $this->ground->db->drop_all_tables();
    $this->ground->add_module('Ground_Drupal');
  }

  public function test_expansion() {
    $this->fixture->populate_database();
    $this->fixture->add_roles_users();
    $this->fixture->add_users();

    $fortress = new Fortress($this->ground);
    $condition = new Anonymous_Condition(function($account, $factors) {
              if (Fortress::user_has_role($account->roles, 'administrator'))
                return true;
              else
                return false;
            });
    $fortress->lay_brick('warrior/name', $condition, 0);
    $this->assertSame(0, $fortress->access($account, 'warrior/name'));
    $role = new stdClass();
    $role->name = 'administrator';
    $account->roles[] = $role;
    $this->assertSame(1, $fortress->access($account, 'warrior/name'));
  }

}

