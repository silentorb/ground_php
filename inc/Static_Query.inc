<?php

// Used to create predefined queries.
class Static_Query {

  public $joins = array();
  public $filters = array();
  public $post_clauses = array();
  public $fields = array();
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

  function add_field($clause) {
    $this->fields[] = $clause;
  }

  function add_join($clause) {
    $this->joins[] = $clause;
  }

  function add_post($clause) {
    $this->post_clauses[] = $clause;
  }

  function create_query($include_links = true, $base_path = '') {
    $query = new Query($this->trellis, $include_links, $base_path);

    $query->filters = $this->filters;
    $query->joins=$this->joins;
    $query->post_clauses = $this->post_clauses;
    
    return $query;
  }

}

