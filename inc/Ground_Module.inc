<?php

class Ground_Module extends Meta_Object {
  
  public $ground;
  
  public function __construct() {
    $this->listen($this, 'connect.ground', 'initialize');
  }
  
  public function initialize ($ground) {
    $this->ground = $ground;
  }
}
