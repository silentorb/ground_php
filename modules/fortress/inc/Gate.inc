<?php

Irrigation::import();

class Gate {

  public $access = 1;
  public $fortress;
  public $max_access = 3;

  public function check($account, $resource, $factors = null) {
    return false;
  }

  public function check_create() {
    return false;
  }

  public function get_access($account, $resource, $factors = null) {
    return $this->access;
  }
}

class Admin_Gate extends Gate {

  public $role;
  public $access = 3;

  public function __construct($role = 'admin') {
    $this->role = $role;
  }

  public function check($account, $resource, $factors = null) {
    return Fortress::user_has_role($account, $this->role);
  }

  public function check_create($account, $trellis) {
    return true;
  }

}

class Public_Trellis_Gate extends Gate {

  public $trellises;

  public function __construct($trellises = array(), $access = 1) {
    $this->trellises = $trellises;
    $this->access = $access;
  }

  public function check($account, $resource, $factors = null) {
    if (is_array($factors) && isset($factors['trellis'])) {
      $trellis = $factors['trellis'];
      if (is_object($trellis))
        $trellis = $trellis->name;
    }
    else {
      $trellis = Fortress::get_resource_trellis($resource);
    }
    if (!$trellis)
      return false;

    return in_array($trellis, $this->trellises);
  }
}

class Object_Creation_Gate extends Gate {

  public $trellises;

  public function __construct($trellises = array()) {
    $this->trellises = $trellises;
  }

  public function check($account, $resource, $factors = null) {
    return false;
  }

  public function check_create($account, $trellis) {
    return in_array($trellis->name, $this->trellises);
  }

}

class Owner_Gate extends Gate {

  public $access = 3;

  public function check($account, $resource, $factors = null) {
    return $account->uid == Fortress::get_owner_id($resource);
  }

}