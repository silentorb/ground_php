<?php

class History_Object {

  public $id;
  public $trellis;
  public $object;
  public $ground;

  public function __construct($object, $trellis, $ground) {
    $this->trellis = $trellis;
    $this->object = $object;
    $this->ground = $ground;
  }

  public function get_latest_update($property) {
    $query = $this->ground->create_query('history_update');
    $query->add_filter("object = $this->id AND property = '$property->name'");
    $query->add_post('ORDER BY commit DESC');
    $query->add_post('LIMIT 1');
    $objects = $query->run();
    return $objects[0];
  }

  public function property_needs_update($property) {
    $latest_update = $this->get_latest_update($property);
    if (!$latest_update)
      return true;

    $value = $this->ground->convert_value($latest_update->value, $property->type);
    if ($value !== $this->object->{$property->name})
      return true;
    
    return false;
  }

  public function update() {
    $trellis = $this->trellis;
    $id = $trellis->get_object_id($this->object);
    $trellis_name = $trellis->name;
    $query = $this->ground->create_query('history_object');
    $query->add_filter("trellis = '$trellis_name' AND object_id = $id");
    $objects = $query->run();
    if (count($objects) == 0) {
      // If there is no record for this object in the history database
      // it will be created and $this->id will be updated.
      $data = array(
          'id' => $this->id,
          'trellis' => $trellis->name,
          'object_id' => $id,
      );
      $update = new Update('history_object', $data, $this->ground);
      $result = $update->run();
      $this->id = $result->seed->id;
      return $result;
    }
  }

}