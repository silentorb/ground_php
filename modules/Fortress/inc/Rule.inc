<?php

namespace ground\fortress;
\Irrigation::import();

class Rule {

  public $resource;
  public $condition;
  public $access;
  public $weight = 0;

  public function __construct($resource, $condition, $access) {
    $this->resource = new \ground\irrigation\Path($resource);
    $this->condition = $condition;
    $this->access = $access;
  }

  public function check($account, $factors = null) {
    if ($this->condition === true) {
      return true;
    }

    return false;
  }

}

