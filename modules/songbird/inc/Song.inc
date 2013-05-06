<?php

class Song {

  public $type;
  public $data;
  public $user;
  public $listeners;

  public function __construct($type, $data, $user, $listeners) {
    $this->type = $type;
    $this->data = $data;
    $this->user = $user;
    $this->listeners = $listeners;
  }

}
