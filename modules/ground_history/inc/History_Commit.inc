<?php

class History_Update {

  public $id;
  public $commit;
  public $trellis;
  public $object;
  public $property;
  public $value;

  public function __construct($commit, $trellis, $object, $property) {
    $this->commit = $commit;
    $this->trellis = $trellis;
    $this->object = $object;
    $this->property = $property;
    
    if (is_object($object))
      $this->value = $object->{$property->name};
    else if (is_array($object))
      $this->value = $object[$property->name];
  }

}

class History_Commit {

  public $ground;
  public $updates = array();

  public function __construct($ground) {
    $this->ground = $ground;
  }

  public function process_object($object, $trellis) {
    foreach ($trellis->all_properties as $property) {

      // We're only concerned with properties that can change.
      if ($property->name == $trellis->primary_key)
        continue;

      $update = new History_Update($this, $trellis, $object, $property);
      $this->updates[] = $update;
    }
  }

  public function process_trellis($trellis) {
    $query = $this->ground->create_query($trellis);
    $result = $query->run();
    foreach ($result->objects as $object) {
      $this->update_object($object, $trellis);
    }
  }

  public function save() {
    
  }

}
