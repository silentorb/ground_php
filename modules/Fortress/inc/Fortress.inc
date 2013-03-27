<?php

class Fortress extends Ground_Module {

  public $rules = array();
  public $ground;

  public function __construct($ground = null) {
    if ($ground)
      $this->ground = $ground;
  }

  public function add_rule($resource, $condition, $allow) {
    $rule = new \ground\fortress\Rule($resource, $condition, $allow);
    $this->rules[] = $rule;
    return $rule;
  }

  public function access($account, $resource, $factors = null) {
    $access = 1;
    foreach ($this->rules as $rule) {
      if ($rule->resource->match($resource) && $rule->check($account, $resource)) {
        $access = $rule->access;
      }
    }

    return $access;
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

namespace ground\fortress;

class Rule {

  public $resource;
  public $condition;
  public $access;
  public $weight = 0;

  public function __construct($resource, $condition, $access) {
    $this->resource = new irrigation\Path($resource);
    $this->condition = $condition;
    $this->access = $access;
  }

  public function check($account, $factors = null) {
    if ($this->condition === true) {
      return true;
    }

    return false;
  }

}

