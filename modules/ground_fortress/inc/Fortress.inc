<?php

require_once dirname(__FILE__) . DIRECTORY_SEPARATOR . 'Brick.inc';

class Fortress extends Ground_Module {

  public $bricks = array();
  public $ground;

  public function __construct($ground = null) {
    if ($ground)
      $this->ground = $ground;
  }

  public function lay_brick($resource, $condition, $allow) {
    $brick = new Brick($resource, $condition, $allow);
    $this->bricks[] = $brick;
    return $brick;
  }

  public function access($account, $resource, $factors = null) {
    $access = 1;
    foreach ($this->bricks as $rule) {
      if ($rule->resource->match($resource) && $rule->check($account, $resource)) {
        $access = $rule->access;
      }
    }

    return $access;
  }

  public static function get_owner_uid($resource) {
    if (!is_object($resource))
      return null;

    if (isset($resource->owner)) {
      $owner - $$resource->owner;
    }
    else if (isset($resource->author)) {
      $owner - $$resource->author;
    }

    if (is_object($owner)) {
      if (isset($owner->id))
        return $owner->id;

      if (isset($owner->uid))
        return $owner->uid;

      return null;
    }

    $owner = (int) $owner;
    if ($owner === 0)
      return null;

    return $owner;
  }

  public static function user_has_role($user, $name) {
    if (!$user->roles)
      return false;

    foreach ($user->roles as $role) {
      if ($role->name == $name)
        return true;
    }

    return false;
  }

}

abstract class Base_Condition {
  
}

class Anonymous_Condition extends Base_Condition {

  public $function;

  public function __construct($function) {
    $this->function = $function;
  }

  public function check($account, $factors) {
    return false;
  }

}

class Method_Condition extends Base_Condition {

  public $target;
  public $method;
  public $factors;

  public function __construct($target, $method, $factors = null) {
    $this->target = $target;
    $this->method = $method;
    $this->factors = $factors;
  }

  public function check($account, $factors) {
    return false;
  }

}

class Pattern_Condition extends Base_Condition {

  public $pattern;
  public $irrigation;

  public function __construct($pattern, $irrigation) {
    $this->pattern = $pattern;
    $this->irrigation = $irrigation;
  }

  public function check($account, $factors) {
    return $irrigation->expand($account, $this->pattern);
  }

}
