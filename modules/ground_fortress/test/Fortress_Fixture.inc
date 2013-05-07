<?php

class Fortress_Fixture {
  public function populate_users() {
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
  }

}
