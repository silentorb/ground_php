<?php

class Fortress_Test extends Ground_Test_Case {
  public function test_expansion() {
    $this->ground->add_module('Ground_Drupal');
    $this->fixture->populate_database();
    $this->fixture->add_roles_users();

    $this->fixture->insert_object('user', array(
        'name' => 'The Cool One',
        'email' => 'cool@nowhere.com',
        'pass' => 'password',
    ));

    $account = $this->fixture->insert_object('user', array(
        'name' => 'The No One',
        'email' => 'nothing@nowhere.com',
        'pass' => 'password2',
        'roles' => array(
            array(
                'name' => 'no one',
            )
        )
            ));


    $fortress = new Fortress($this->ground);
    $condition = new Anonymous_Condition(function($account, $factors) {
                      if (Fortress::user_has_role($account->roles, 'administrator'))
                        return true;
                      else
                        return false;
                    });
    $fortress->add_rule('warrior/name', $condition, 0);
    $this->assertSame(0, $fortress->access($account, 'warrior/name'));
    $role = new stdClass();
    $role->name = 'administrator';
    $account->roles[] = $role;
    $this->assertSame(1, $fortress->access($account, 'warrior/name'));
  }

}

