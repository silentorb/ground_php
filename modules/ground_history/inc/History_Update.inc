<?php

class History_Update {

  public $id;
  public $commit;
  public $trellis;
  public $object;
  public $property;
  public $value;

  public function __construct($commit, $object, $property, $trellis = null) {
    $this->commit = $commit;

    $this->object = $object;
    $this->property = $property;

    if ($trellis) {
      $this->trellis = $trellis;
    }
    else {
      $object = $property->target;
    }

    if (is_object($object))
      $this->value = $object->{$property->name};
    else if (is_array($object))
      $this->value = $object[$property->name];
  }

  public function save($ground) {
    $row = new stdClass();
    $row->commit = $this->commit->id;
    $row->trellis = $this->trellis->name;
    // !!! This is a quick fix with id.  Will need dynamic key later.
    $row->object = $this->object->id;
    $row->property = $this->property->name;
    $row->value = $this->value;

    $update = new Update('history_update', $row, $ground);
    // !!! More quick id stuff.
    $this->id = $row->id;
    return $update->run();
  }

}