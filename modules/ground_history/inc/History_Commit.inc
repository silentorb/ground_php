<?php

class History_Commit {

  public $ground;
  public $updates = array();
  public $timestamp;
  public $history;

  public function __construct($ground) {
    $this->ground = $ground;
    $this->history = $ground->modules['History'];
  }

  public function process_object($object, $trellis) {
    $history_object = $this->history->get_object($object, $trellis);
    $properties = $trellis->get_all_properties();
    foreach ($properties as $property) {

      // We're only concerned with properties that can change.
      if ($property->name == $trellis->primary_key)
        continue;

      if ($property->type == 'list')
        continue;

      if (!$history_object->property_needs_update($property))
        continue;
      
      $update = new History_Update($this, $object, $property, $trellis);
      $this->updates[] = $update;
    }
  }

  public function process_trellis($trellis) {
    $query = $this->ground->create_query($trellis);
    $result = $query->run_as_service();
    foreach ($result->objects as $object) {
      $this->process_object($object, $trellis);
    }
  }

  public function save() {
    global $user;
    if (is_object($user)) {
      $this->user = $user->uid;
    }

    $this->timestamp = time();
    $update = new Update('history_commit', $this, $this->ground);
    $update->run();

    foreach ($this->updates as $update) {
      $update->save($this->ground);
    }
  }

}
