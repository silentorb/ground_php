<?php

class Wall {

  public $fortress;

  public function has_gate($account, $factors = array()) {
    return false;
  }

  public function repel() {
    return 'Access denied.';
  }

}

class Resource_Wall extends Wall{

  public $level;
  public $guide;

  public function __construct($level, $guide) {
    $this->level = $level;
    $this->guide = $guide;
  }

  public function has_gate($account, $request) {
    $resource = $this->guide->get($request);
    if (!$resource) {
      throw new Exception('Resource_Wall could not find the requested resource.');
    }
    
    $access = $this->fortress->access($account, $resource);
    return $access >= $this->level;
  }

  public function repel() {
    return 'Permission denied.';
  }
}

class Drupal_Wall extends Wall {

  public $permission;

  public function __construct($permission) {
    $this->permission = $permission;
  }

  public function has_gate($account, $factors = array()) {
    return user_access($this->permission, $account);
  }

}