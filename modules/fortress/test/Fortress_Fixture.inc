<?php

class Fortress_Fixture extends Ground_Fixture {
  public function populate_users() {
    $this->insert_object('user', array(
        'name' => 'The Cool One',
        'email' => 'cool@nowhere.com',
        'pass' => 'password',
    ));

    $account = $this->insert_object('user', array(
        'name' => 'The No One',
        'email' => 'nothing@nowhere.com',
        'pass' => 'password2',
        'roles' => array(
            array(
                'name' => 'no one',
            )
        )
        ));
  }

}

Fortress::load();

class Dummy_Gate extends Gate {
  public $check_result = 3;
  
  public function check($account, $resource, $factors = null) {
    return $this->check_result;
  }
}