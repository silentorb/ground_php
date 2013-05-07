<?php

Irrigation::import();

class Brick {

  public $access = 1;

//  public $weight = 0;

  public function check($account, $resource, $factors = null) {
    return true;
  }

}

class Admin_Brick extends Brick {

  public $role;
  public $access = 3;
  
  public function __construct($role = 'admin') {
    $this->role = 'admin';
  }

  public function check($account, $resource, $factors = null) {
    return Fortress::user_has_role($account, $this->role);
  }

}

class Public_Trellis_Brick extends Brick {

  public $trellises;

  public function __construct($trellises = array()) {
    $this->trellises = $trellises;
  }

  public function check($account, $resource, $factors = null) {
    return in_array($resource->type, $this->trellises);
  }

}

class Owner_Brick extends Brick {

  public $access = 3;

  public function check($account, $resource, $factors = null) {
    return $account->uid == Fortress::get_owner_uid ($resource->owner->uid);
  }

}