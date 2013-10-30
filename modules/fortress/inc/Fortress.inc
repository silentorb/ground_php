<?php

require_once dirname(__FILE__) . DIRECTORY_SEPARATOR . 'Wall.inc';
require_once dirname(__FILE__) . DIRECTORY_SEPARATOR . 'Gate.inc';
require_once dirname(__FILE__) . DIRECTORY_SEPARATOR . 'Guide.inc';

class Fortress extends Ground_Module {

  public $gates = array();
  public $ground;
  // Set this to have a global, default user.
  public $account;

  public function __construct($ground = null) {
    if ($ground)
      $this->ground = $ground;

    $ground->fortress = $this;
  }

  public function lay_gate($gate) {
    $this->gates[] = $gate;
    $gate->fortress = $this;
    return $gate;
  }

  public function access($account, $resource, $factors = null, $max = 3) {
    if (!$account) {
      $account = $this->account;
    }

    if (!$account || !$resource)
      return 0;

    $access = 0;
    foreach ($this->gates as $gate) {
      if ($gate->max_access > $access) {
//        $new_access = $gate->get_access($account, $resource, $factors);
        $result = $gate->check($account, $resource, $factors);
        if ($result === true) {
          $access = $gate->access;
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

  public function channel_access($account, $channel, $request) {
    if (!$account) {
      $account = $this->account;
    }

    if (!$account)
      throw new Exception('Missing account in channel reference.');

    foreach ($channel->walls as $wall) {
      if (!$wall->has_gate($account, $request)) {
        return $wall;
      }
    }

    return true;
  }

  public function creation_access($account, $trellis) {
    if (!$account) {
      $account = $this->account;
    }

    if (!$account || !$trellis)
      return 0;

    $access = 0;
    foreach ($this->gates as $gate) {
      if ($gate->access > $access && $gate->check_create($account, $trellis)) {
        $access = $gate->access;
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

  public static function load() {
    // Dummy function to trigger the included files at the top of this file.
  }


 function get_authorized_properties($properties, $base_path, $account = null, $access = 1) {
    $result = array();
    foreach ($properties as $property) {
      if ($property->private)
        continue;

      $property_access = $this->access($account, $base_path . $property->name);
      if ($property_access >= $access) {
        $result[$property->name] = $property;
      }
    }
    return $result;
  }
}
