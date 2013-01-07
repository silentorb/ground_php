<?php

class History_Object {

  public $id;
  public $trellis;
  public $object_id;
  public $target;

  public function __construct($object, $trellis) {
    $this->trellis = $trellis;
    $this->target = $object;
    $this->object_id = $trellis->get_object_id($object);
  }

}

class History extends Ground_Module {

  public $objects = array();

  public function initialize($ground) {
    parent::initialize($ground);
    $this->listen($ground, 'updated', 'on_updated');
    $path = drupal_get_path('module', 'ground_history');
    $this->ground->load_schema_from_file($path . '/ground_history.json');
  }

  function on_updated($seed, $trellis) {
    
  }

  function auto_commit() {
    $commit = new History_Commit();


    $commit->save();
  }

  public function get_object($object, $trellis) {
    $key = $trellis->name . '.' . $trellis->get_object_id($object);
    if (array_key_exists($key, $this->objects)) {
      return $this->objects[$key];
    }
    else {
      $history_object = new History_Object($object, $trellis);
      $this->objects[$key] = $history_object;
      return $history_object;
    }
  }

}

