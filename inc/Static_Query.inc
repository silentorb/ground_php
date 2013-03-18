<?php

// Used to create predefined queries.
class Static_Query {

  public $filters = array();
  public $name;
  public $trellis;
  public $is_static = true;

//  public $include_links;

  public function __construct($name, $trellis, $include_links = true) {
    $this->name = $name;
    $this->trellis = $trellis;
//    $this->include_links = $include_links;
  }

  function add_filter($clause) {
    $this->filters[] = $clause;
  }

  function create_query($include_links = true, $base_path = '') {
    $query = new Query($this->trellis, $include_links, $base_path);

    $query->filters = $this->filters;
    return $query;
  }

}

