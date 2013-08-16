<?php

class Wall {

  public $fortress;

  public function collide($account, $factors = array()) {
    return false;
  }

  public function repel() {
    return 'Access denied.';
  }

}

class Resource_Wall extends Wall{

  public $level;
  public $resource_key;

  public function __construct($level, $resource_key = 'resource') {
    $this->level = $level;
    $this->resource_key = $resource_key;
  }

  public function collide($account, $factors = array()) {
    $access = $this->fortress->access($account, $factors[$this->resource_key]);
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

  public function collide($account, $factors = array()) {
    return user_access($this->permission, $account);
  }

}