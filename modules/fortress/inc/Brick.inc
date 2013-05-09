<?php

Irrigation::import();

class Brick {

  public $access = 1;

//  public $weight = 0;

  public function check($account, $resource, $factors = null) {
    return false;
  }

}

class Admin_Brick extends Brick {

  public $role;
  public $access = 3;

  public function __construct($role = 'admin') {
    $this->role = $role;
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
    $trellis = Fortress::get_resource_trellis($resource);
    if (!$trellis)
      return false;

    return in_array($trellis, $this->trellises);
  }

}

class Owner_Brick extends Brick {

  public $access = 3;

  public function check($account, $resource, $factors = null) {
    return $account->uid == Fortress::get_owner_id($resource);
  }

}