<?php

class Guide {
  public function get($request) {
    return null;
  }

}

class Resource_Guide extends Guide {

  public $index;

  public function __construct($index) {
    $this->index = $index;
  }

  public function get($request) {
    return $request->resources[$this->index];
  }

}