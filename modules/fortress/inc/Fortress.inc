<?php

require_once dirname(__FILE__) . DIRECTORY_SEPARATOR . 'Brick.inc';

class Fortress extends Ground_Module {

  public $bricks = array();
  public $ground;
  // Set this to have a global, default user.
  public $account;

  public function __construct($ground = null) {
    if ($ground)
      $this->ground = $ground;

    $ground->fortress = $this;
  }

  public function lay_brick($brick) {
    $this->bricks[] = $brick;
    $brick->fortress = $this;
    return $brick;
  }

  public function access($account, $resource, $factors = null, $max = 3) {
    if (!$account) {
      $account = $this->account;
    }

    if (!$account || !$resource)
      return 0;

    $access = 0;
    foreach ($this->bricks as $brick) {
      if ($brick->max_access > $access) {
//        $new_access = $brick->get_access($account, $resource, $factors);
        $result = $brick->check($account, $resource, $factors);
        if ($result === true) {
          $access = $brick->access;
        }
        else if (is_int($result)) {
          if ($result > $access) {
            $access = $result;
          }
        }
      }
      if ($access >= $max)
        break;
    }

    return $access;
  }

  public function creation_access($account, $trellis) {
    if (!$account) {
      $account = $this->account;
    }

    if (!$account || !$trellis)
      return 0;

    $access = 0;
    foreach ($this->bricks as $brick) {
      if ($brick->access > $access && $brick->check_create($account, $trellis)) {
        $access = $brick->access;
      }
    }

    return $access;
  }

  static function get_owner_id($resource) {
    if (!is_object($resource))
      return null;

    if (isset($resource->owner)) {
      $owner = $resource->owner;
    }
    else if (isset($resource->author)) {
      $owner = $resource->author;
    }
    else if (isset($resource->uid)) {
      $owner = $resource->uid;
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

  static function get_resource_trellis($resource) {
    if (!$resource)
      return null;

    if (isset($resource->trellis)) {
      $trellis = $resource->trellis;
    }
    else if (isset($resource->type)) {
      $trellis = $resource->type;
    }
    else {
      return null;
    }

    if (is_object($trellis)) {
      if (!isset($trellis->name))
        return null;

      $trellis = $trellis->name;
    }

    return $trellis;
  }

  public static function user_has_role($user, $name) {
    if (!$user->roles)
      return false;

    foreach ($user->roles as $role) {
      if (is_object($role)) {
        if ($role->name == $name)
          return true;
      }
      if (is_array($role)) {
        if ($role[name] == $name)
          return true;
      }

      if ($role == $name)
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
