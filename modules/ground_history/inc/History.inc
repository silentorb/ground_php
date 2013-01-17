<?php

class History_Object {

  public $id;
  public $trellis;
  public $object;

  public function __construct($object, $trellis) {
    $this->trellis = $trellis;
    $this->object = $object;
  }

  public function update($ground) {
    $id = $trellis->get_object_id($this->object);
    $trellis_name =$this->trellis->name;
    $query = $ground->create_query('history_object');
    $query->add_filter("trellis = $trellis_name AND object_id = $id");
    $objects = $query->run();
    if (count($objects) == 0) {
      // If there is no record for this object in the history database
      // it will be created and $this->id will be updated.
      $update = new Update('history_object', $this, $ground);
      return $update->run();
    }
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
      // Save the history_object in the database if it's not already there.
      $history_object->update($this->ground);
      $this->objects[$key] = $history_object;
      return $history_object;
    }
  }

}

